// © 2025 Kamenyari. All rights reserved.

#include "Traversal/NotoTraversalComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/NotoCharacterAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "AnimationWarpingLibrary.h"
#include "Character/NotoPlayerPawnComponent.h"
#include "Components/CapsuleComponent.h"
#include "Development/NotoGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "Traversal/NotoTraversalSurface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoTraversalComponent)

namespace NotoTraversal
{
	const FName FrontLedgeWarpTarget(TEXT("FrontLedge"));
	const FName BackLedgeWarpTarget(TEXT("BackLedge"));
	const FName BackFloorWarpTarget(TEXT("BackFloor"));
	const FName DistanceFromLedgeCurve(TEXT("Distance_From_Ledge"));
	constexpr float SurfaceClearance = 2.0f;
	constexpr float FrontLedgeVerticalOffset = 0.5f;
	constexpr float BackFloorProbeDistance = 50.0f;
}

UNotoTraversalComponent::UNotoTraversalComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UNotoTraversalComponent::TryTraversalAction(const FNotoTraversalCheckInputs& Inputs, FNotoTraversalCheckResult& OutResult)
{
	OutResult = FNotoTraversalCheckResult();
	return QueryTraversalSurface(Inputs, OutResult);
}

FNotoTraversalMontageChooserInput UNotoTraversalComponent::BuildMontageChooserInput(const FNotoTraversalCheckResult& Result, const FPoseHistoryReference& PoseHistory) const
{
	FNotoTraversalMontageChooserInput Input;
	Input.ActionType = Result.ActionType;
	Input.bHasFrontLedge = Result.bHasFrontLedge;
	Input.bHasBackLedge = Result.bHasBackLedge;
	Input.bHasBackFloor = Result.bHasBackFloor;
	Input.ObstacleHeight = Result.ObstacleHeight;
	Input.ObstacleDepth = Result.ObstacleDepth;
	Input.BackLedgeHeight = Result.BackLedgeHeight;
	Input.PoseHistory = PoseHistory;

	if (const ACharacter* Character = GetOwnerCharacter())
	{
		Input.DistanceToLedge = FVector::Distance(Result.FrontLedgeLocation, Character->GetMesh()->GetComponentLocation());
		Input.Speed = Character->GetVelocity().Size2D();
		if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Input.MovementMode = Movement->IsFalling() ? ENotoLocomotionMovementMode::InAir : ENotoLocomotionMovementMode::OnGround;
		}
		if (const UNotoPlayerPawnComponent* PlayerPawnComponent = Character->FindComponentByClass<UNotoPlayerPawnComponent>())
		{
			Input.Gait = PlayerPawnComponent->GetResolvedGait();
		}
	}

	return Input;
}

FNotoTraversalCheckResult UNotoTraversalComponent::ApplyMontageChooserOutput(FNotoTraversalCheckResult Result, const FNotoTraversalMontageChooserOutput& Selection) const
{
	Result.ActionType = Selection.ActionType;
	Result.ChosenMontage = Selection.Montage;
	Result.StartTime = Selection.MontageStartTime;
	Result.PlayRate = Selection.PlayRate;
	return Result;
}

bool UNotoTraversalComponent::StartTraversal(const FNotoTraversalCheckResult& Result)
{
	ACharacter* Character = GetOwnerCharacter();
	UAnimInstance* AnimInstance = Character ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (bTraversalActive || !Character || !AnimInstance || !Result.ChosenMontage)
	{
		return false;
	}

	const float MontageDuration = AnimInstance->Montage_Play(Result.ChosenMontage, Result.PlayRate, EMontagePlayReturnType::MontageLength, Result.StartTime);
	if (MontageDuration <= 0.0f)
	{
		return false;
	}

	TraversalResult = Result;
	ActiveMontage = Result.ChosenMontage;
	SetTraversalActionActive(true);
	SetWarpTargets(TraversalResult);

	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent(); Capsule && TraversalResult.HitComponent)
	{
		Capsule->IgnoreComponentWhenMoving(TraversalResult.HitComponent, true);
	}
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Flying);
		Movement->bIgnoreClientMovementErrorChecksAndCorrection = true;
		Movement->bServerAcceptClientAuthoritativePosition = true;
	}

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ThisClass::HandleMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, ActiveMontage);
	return true;
}

void UNotoTraversalComponent::StopTraversal(float BlendOutTime)
{
	if (!bTraversalActive)
	{
		return;
	}

	if (ACharacter* Character = GetOwnerCharacter())
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(BlendOutTime, ActiveMontage);
		}
	}
	FinishTraversal();
}

void UNotoTraversalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReplicationRestoreTimer);
	}
	FinishTraversal();
	Super::EndPlay(EndPlayReason);
}

bool UNotoTraversalComponent::QueryTraversalSurface(const FNotoTraversalCheckInputs& Inputs, FNotoTraversalCheckResult& OutResult) const
{
	ACharacter* Character = GetOwnerCharacter();
	if (!Character || Inputs.TraceRadius <= 0.0f || Inputs.TraceHalfHeight <= 0.0f)
	{
		return false;
	}

	const FVector CharacterLocation = Character->GetActorLocation();
	const FVector TraceStart = CharacterLocation + Inputs.TraceOriginOffset;
	const FVector TraceEnd = TraceStart + Inputs.TraceForwardDirection.GetSafeNormal() * Inputs.TraceForwardDistance + Inputs.TraceEndOffset;
	const TArray<AActor*> IgnoredActors{Character};
	FHitResult SurfaceHit;
	if (!UKismetSystemLibrary::CapsuleTraceSingle(this, TraceStart, TraceEnd, Inputs.TraceRadius, Inputs.TraceHalfHeight,
		ETraceTypeQuery::TraceTypeQuery3, false, IgnoredActors, EDrawDebugTrace::None, SurfaceHit, true))
	{
		return false;
	}

	AActor* SurfaceActor = SurfaceHit.GetActor();
	if (!SurfaceActor || !SurfaceActor->Implements<UNotoTraversalSurface>())
	{
		return false;
	}

	FNotoTraversalLedgeData LedgeData;
	INotoTraversalSurface::Execute_GetTraversalLedgeData(SurfaceActor, SurfaceHit.ImpactPoint, CharacterLocation, LedgeData);
	if (!LedgeData.bHasFrontLedge)
	{
		return false;
	}

	OutResult.bHasFrontLedge = true;
	OutResult.FrontLedgeLocation = LedgeData.FrontLedgeTransform.GetLocation();
	OutResult.FrontLedgeNormal = LedgeData.FrontLedgeTransform.GetUnitAxis(EAxis::Z);
	OutResult.bHasBackLedge = LedgeData.bHasBackLedge;
	OutResult.BackLedgeLocation = LedgeData.BackLedgeTransform.GetLocation();
	OutResult.BackLedgeNormal = LedgeData.BackLedgeTransform.GetUnitAxis(EAxis::Z);
	OutResult.HitComponent = SurfaceHit.GetComponent();
	if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
	{
		if (UNotoCharacterAnimInstance* NotoAnimInstance = Cast<UNotoCharacterAnimInstance>(AnimInstance))
		{
			NotoAnimInstance->SetTraversalInteractionTransform(FTransform(FRotationMatrix::MakeFromZ(OutResult.FrontLedgeNormal).Rotator(), OutResult.FrontLedgeLocation));
		}
	}

	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule)
	{
		return false;
	}

	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector FrontClearanceLocation = OutResult.FrontLedgeLocation
		+ OutResult.FrontLedgeNormal * (CapsuleRadius + NotoTraversal::SurfaceClearance)
		+ FVector::UpVector * (CapsuleHalfHeight + NotoTraversal::SurfaceClearance);
	FHitResult FrontClearanceHit;
	const bool bFrontBlocked = UKismetSystemLibrary::CapsuleTraceSingle(this, CharacterLocation, FrontClearanceLocation, CapsuleRadius, CapsuleHalfHeight,
		ETraceTypeQuery::TraceTypeQuery1, false, IgnoredActors, EDrawDebugTrace::None, FrontClearanceHit, true);
	if (!bFrontBlocked)
	{
		OutResult.ObstacleHeight = FMath::Abs((CharacterLocation - FVector::UpVector * CapsuleHalfHeight - OutResult.FrontLedgeLocation).Z);
		return true;
	}

	if (!OutResult.bHasBackLedge)
	{
		return false;
	}

	const FVector BackClearanceLocation = OutResult.BackLedgeLocation
		+ OutResult.BackLedgeNormal * (CapsuleRadius + NotoTraversal::SurfaceClearance)
		+ FVector::UpVector * (CapsuleHalfHeight + NotoTraversal::SurfaceClearance);
	FHitResult TopSweepHit;
	const bool bTopBlocked = UKismetSystemLibrary::CapsuleTraceSingle(this, FrontClearanceLocation, BackClearanceLocation, CapsuleRadius, CapsuleHalfHeight,
		ETraceTypeQuery::TraceTypeQuery1, false, IgnoredActors, EDrawDebugTrace::None, TopSweepHit, true);
	if (!bTopBlocked)
	{
		OutResult.ObstacleDepth = FVector::Dist2D(OutResult.BackLedgeLocation, OutResult.FrontLedgeLocation);
		return true;
	}

	FHitResult BackFloorHit;
	const bool bHasBackFloor = UKismetSystemLibrary::CapsuleTraceSingle(this, BackClearanceLocation,
		BackClearanceLocation - FVector::UpVector * NotoTraversal::BackFloorProbeDistance, CapsuleRadius, CapsuleHalfHeight,
		ETraceTypeQuery::TraceTypeQuery1, false, IgnoredActors, EDrawDebugTrace::None, BackFloorHit, true);
	OutResult.bHasBackFloor = bHasBackFloor;
	if (bHasBackFloor)
	{
		OutResult.BackFloorLocation = BackFloorHit.ImpactPoint;
		OutResult.BackLedgeHeight = FMath::Abs((BackFloorHit.ImpactPoint - OutResult.BackLedgeLocation).Z);
	}
	else
	{
		OutResult.ObstacleDepth = FVector::Dist2D(TopSweepHit.ImpactPoint, OutResult.FrontLedgeLocation);
	}

	return true;
}

void UNotoTraversalComponent::SetWarpTargets(const FNotoTraversalCheckResult& Result) const
{
	UMotionWarpingComponent* MotionWarping = GetMotionWarpingComponent();
	if (!MotionWarping)
	{
		return;
	}

	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(NotoTraversal::FrontLedgeWarpTarget,
		Result.FrontLedgeLocation + FVector::UpVector * NotoTraversal::FrontLedgeVerticalOffset, (-Result.FrontLedgeNormal).Rotation());

	if (Result.ActionType == ENotoTraversalActionType::Hurdle || Result.ActionType == ENotoTraversalActionType::Vault)
	{
		TArray<FMotionWarpingWindowData> Windows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(Result.ChosenMontage, NotoTraversal::BackLedgeWarpTarget, Windows);
		if (!Windows.IsEmpty())
		{
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(NotoTraversal::BackLedgeWarpTarget, Result.BackLedgeLocation, (-Result.BackLedgeNormal).Rotation());
		}
		else
		{
			MotionWarping->RemoveWarpTarget(NotoTraversal::BackLedgeWarpTarget);
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(NotoTraversal::BackLedgeWarpTarget);
	}

	if (Result.ActionType == ENotoTraversalActionType::Vault)
	{
		TArray<FMotionWarpingWindowData> BackLedgeWindows;
		TArray<FMotionWarpingWindowData> BackFloorWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(Result.ChosenMontage, NotoTraversal::BackLedgeWarpTarget, BackLedgeWindows);
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(Result.ChosenMontage, NotoTraversal::BackFloorWarpTarget, BackFloorWindows);
		if (!BackLedgeWindows.IsEmpty() && !BackFloorWindows.IsEmpty())
		{
			float AnimatedBackLedgeDistance = 0.0f;
			float AnimatedBackFloorDistance = 0.0f;
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(Result.ChosenMontage, NotoTraversal::DistanceFromLedgeCurve, BackLedgeWindows[0].StartTime, AnimatedBackLedgeDistance);
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(Result.ChosenMontage, NotoTraversal::DistanceFromLedgeCurve, BackFloorWindows[0].StartTime, AnimatedBackFloorDistance);
			const float DistanceOffset = FMath::Abs(AnimatedBackLedgeDistance - AnimatedBackFloorDistance);
			const FVector TargetLocation = Result.BackFloorLocation + Result.BackLedgeNormal * DistanceOffset;
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(NotoTraversal::BackFloorWarpTarget, TargetLocation, (-Result.BackLedgeNormal).Rotation());
		}
		else
		{
			MotionWarping->RemoveWarpTarget(NotoTraversal::BackFloorWarpTarget);
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(NotoTraversal::BackFloorWarpTarget);
	}
}

void UNotoTraversalComponent::SetTraversalActionActive(bool bActive)
{
	if (bTraversalActive == bActive)
	{
		return;
	}

	bTraversalActive = bActive;
	if (IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* AbilitySystem = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			AbilitySystem->AddLooseGameplayTag(NotoGameplayTags::State_Traversing, bActive ? 1 : -1, EGameplayTagReplicationState::None);
		}
	}
}

void UNotoTraversalComponent::FinishTraversal()
{
	if (!bTraversalActive)
	{
		return;
	}

	if (ACharacter* Character = GetOwnerCharacter())
	{
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent(); Capsule && TraversalResult.HitComponent)
		{
			Capsule->IgnoreComponentWhenMoving(TraversalResult.HitComponent, false);
		}
	}
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ReplicationRestoreTimer, this, &ThisClass::RestoreReplicationBehavior, 0.2f, false);
		}
		else
		{
			RestoreReplicationBehavior();
		}
	}

	SetTraversalActionActive(false);
	ActiveMontage = nullptr;
}

void UNotoTraversalComponent::RestoreReplicationBehavior()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bIgnoreClientMovementErrorChecksAndCorrection = false;
		Movement->bServerAcceptClientAuthoritativePosition = false;
	}
}

void UNotoTraversalComponent::HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ActiveMontage)
	{
		FinishTraversal();
	}
}

ACharacter* UNotoTraversalComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

UMotionWarpingComponent* UNotoTraversalComponent::GetMotionWarpingComponent() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UMotionWarpingComponent>() : nullptr;
}

UCharacterMovementComponent* UNotoTraversalComponent::GetCharacterMovement() const
{
	return GetOwnerCharacter() ? GetOwnerCharacter()->GetCharacterMovement() : nullptr;
}
