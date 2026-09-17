// © 2025 Kamenyari. All rights reserved.

#include "Ragdoll/NotoRagdollComponent.h"

#include "Combat/NotoHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Development/NotoGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoRagdollComponent)

UNotoRagdollComponent::UNotoRagdollComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNotoRagdollComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* Character = GetOwnerCharacter())
	{
		HealthComponent = Character->FindComponentByClass<UNotoHealthComponent>();
		if (HealthComponent.IsValid())
		{
			HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::HandleDeathStarted);
		}
	}
}

void UNotoRagdollComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HealthComponent.IsValid())
	{
		HealthComponent->OnDeathStarted.RemoveDynamic(this, &ThisClass::HandleDeathStarted);
	}
	HealthComponent.Reset();
	if (RagdollSnapshot.bIsRagdoll)
	{
		StopRagdoll();
	}
	Super::EndPlay(EndPlayReason);
}

bool UNotoRagdollComponent::StartRagdoll(const FVector Impulse, const ENotoRagdollInjuryState InjuryState)
{
	if (RagdollSnapshot.bIsRagdoll)
	{
		return false;
	}

	ACharacter* Character = GetOwnerCharacter();
	USkeletalMeshComponent* Mesh = GetMesh();
	UCapsuleComponent* Capsule = GetCapsule();
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Character || !Mesh || !Capsule || !Movement)
	{
		return false;
	}
	if (Mesh->GetBoneIndex(RagdollRootBone) == INDEX_NONE)
	{
		return false;
	}

	PreviousMeshCollisionProfile = Mesh->GetCollisionProfileName();
	PreviousCapsuleCollisionProfile = Capsule->GetCollisionProfileName();
	PreviousCapsuleCollisionEnabled = Capsule->GetCollisionEnabled();
	PreviousMovementMode = Movement->MovementMode;

	Movement->DisableMovement();
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionProfileName(RagdollCollisionProfile);
	Mesh->SetAllBodiesSimulatePhysics(true);
	Mesh->SetAllBodiesPhysicsBlendWeight(1.0f);
	Mesh->WakeAllRigidBodies();
	if (!Impulse.IsNearlyZero())
	{
		Mesh->AddImpulseToAllBodiesBelow(Impulse, RagdollRootBone, true);
	}

	RagdollSnapshot = {};
	RagdollSnapshot.bIsRagdoll = true;
	RagdollSnapshot.InjuryState = InjuryState;
	RagdollSnapshot.ImpactDirection = FVector2D(Impulse.X, Impulse.Y).GetSafeNormal();
	SetRagdollGameplayState(true);
	SetComponentTickEnabled(true);
	UpdateSnapshot(0.0f);
	return true;
}

void UNotoRagdollComponent::StopRagdoll()
{
	if (!RagdollSnapshot.bIsRagdoll)
	{
		return;
	}

	RestoreCharacterPhysics();
	SetRagdollGameplayState(false);
	RagdollSnapshot = {};
	SetComponentTickEnabled(false);
}

bool UNotoRagdollComponent::ExitRagdollForRecovery()
{
	if (!CanRecover())
	{
		return false;
	}
	if (bAlignCharacterToRecoveryFacing)
	{
		if (ACharacter* Character = GetOwnerCharacter(); !RagdollSnapshot.RecoveryFacingDirection.IsNearlyZero())
		{
			Character->SetActorRotation(RagdollSnapshot.RecoveryFacingDirection.Rotation());
		}
	}

	StopRagdoll();
	return true;
}

void UNotoRagdollComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateSnapshot(DeltaTime);
}

void UNotoRagdollComponent::HandleDeathStarted(const FNotoDamageEvent& DamageEvent)
{
	if (bRagdollOnDeath)
	{
		StartRagdoll();
	}
}

ACharacter* UNotoRagdollComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

USkeletalMeshComponent* UNotoRagdollComponent::GetMesh() const
{
	if (const ACharacter* Character = GetOwnerCharacter())
	{
		return Character->GetMesh();
	}
	return nullptr;
}

UCapsuleComponent* UNotoRagdollComponent::GetCapsule() const
{
	if (const ACharacter* Character = GetOwnerCharacter())
	{
		return Character->GetCapsuleComponent();
	}
	return nullptr;
}

UCharacterMovementComponent* UNotoRagdollComponent::GetCharacterMovement() const
{
	if (const ACharacter* Character = GetOwnerCharacter())
	{
		return Character->GetCharacterMovement();
	}
	return nullptr;
}

void UNotoRagdollComponent::UpdateSnapshot(const float DeltaTime)
{
	if (!RagdollSnapshot.bIsRagdoll)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = GetMesh();
	ACharacter* Character = GetOwnerCharacter();
	UCapsuleComponent* Capsule = GetCapsule();
	if (!Mesh || !Character || !Capsule)
	{
		StopRagdoll();
		return;
	}
	if (Mesh->GetBoneIndex(RagdollRootBone) == INDEX_NONE)
	{
		StopRagdoll();
		return;
	}

	const FTransform RagdollRootTransform = Mesh->GetBoneTransform(RagdollRootBone);
	const FVector RagdollLocation = RagdollRootTransform.GetLocation();
	const FVector RagdollVelocity = Mesh->GetPhysicsLinearVelocity(RagdollRootBone);
	RagdollSnapshot.TimeSinceImpact += DeltaTime;
	RagdollSnapshot.Speed = RagdollVelocity.Size();
	RagdollSnapshot.RollAmount = RagdollRootTransform.GetRelativeTransform(Mesh->GetComponentTransform()).Rotator().Roll;

	const FVector FacingDirection = RagdollRootTransform.GetUnitAxis(RagdollFacingAxis);
	const FVector RightDirection = RagdollRootTransform.GetUnitAxis(RagdollRightAxis);
	RagdollSnapshot.FacingUpAmount = FVector::DotProduct(FacingDirection, FVector::UpVector);
	if (RagdollSnapshot.FacingUpAmount >= FaceUpDotThreshold)
	{
		RagdollSnapshot.RecoveryOrientation = ENotoRagdollRecoveryOrientation::FaceUp;
	}
	else if (RagdollSnapshot.FacingUpAmount <= -FaceUpDotThreshold)
	{
		RagdollSnapshot.RecoveryOrientation = ENotoRagdollRecoveryOrientation::FaceDown;
	}
	else
	{
		RagdollSnapshot.RecoveryOrientation = FVector::DotProduct(RightDirection, FVector::UpVector) >= 0.0f
			? ENotoRagdollRecoveryOrientation::LeftSide
			: ENotoRagdollRecoveryOrientation::RightSide;
	}
	RagdollSnapshot.bCanRecover = RagdollSnapshot.TimeSinceImpact >= MinimumRagdollTimeBeforeRecovery
		&& RagdollSnapshot.Speed <= RecoverySpeedThreshold;
	FVector RecoveryFacingDirection = RagdollRootTransform.GetUnitAxis(RagdollRecoveryForwardAxis);
	RecoveryFacingDirection.Z = 0.0f;
	RagdollSnapshot.RecoveryFacingDirection = RecoveryFacingDirection.Normalize()
		? RecoveryFacingDirection
		: Character->GetActorForwardVector();
	Character->SetActorLocation(RagdollLocation + FVector::UpVector * Capsule->GetScaledCapsuleHalfHeight(), false);
}

void UNotoRagdollComponent::RestoreCharacterPhysics()
{
	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		Mesh->SetAllBodiesSimulatePhysics(false);
		Mesh->SetAllBodiesPhysicsBlendWeight(0.0f);
		if (!PreviousMeshCollisionProfile.IsNone())
		{
			Mesh->SetCollisionProfileName(PreviousMeshCollisionProfile);
		}
	}
	if (UCapsuleComponent* Capsule = GetCapsule())
	{
		Capsule->SetCollisionEnabled(PreviousCapsuleCollisionEnabled);
		if (!PreviousCapsuleCollisionProfile.IsNone())
		{
			Capsule->SetCollisionProfileName(PreviousCapsuleCollisionProfile);
		}
	}
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(PreviousMovementMode);
	}
}

void UNotoRagdollComponent::SetRagdollGameplayState(const bool bActive)
{
	if (IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* AbilitySystem = AbilitySystemOwner->GetAbilitySystemComponent())
		{
			AbilitySystem->AddLooseGameplayTag(NotoGameplayTags::State_Ragdoll, bActive ? 1 : -1, EGameplayTagReplicationState::None);
		}
	}
}
