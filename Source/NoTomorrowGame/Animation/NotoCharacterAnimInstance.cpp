// © 2025 Kamenyari. All rights reserved.

#include "Animation/NotoCharacterAnimInstance.h"

#include "Animation/NotoHeldItemAnimationProfile.h"
#include "Character/NotoCharacter.h"
#include "Ragdoll/NotoRagdollComponent.h"
#include "Traversal/NotoTraversalComponent.h"
#include "Character/NotoEquippedItemActor.h"
#include "Character/NotoEquippedItemComponent.h"
#include "Character/NotoPlayerPawnComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoCharacterAnimInstance)

namespace
{
	bool HasVariantChanged(const FNotoHeldItemAnimSnapshot& Previous, const FNotoHeldItemAnimSnapshot& Current)
	{
		return Previous.bHasHeldItemAnimation != Current.bHasHeldItemAnimation
			|| Previous.PoseMode != Current.PoseMode
			|| Previous.bCrouched != Current.bCrouched
			|| Previous.StanceSequence != Current.StanceSequence
			|| Previous.AimOffset != Current.AimOffset;
	}
}

void UNotoCharacterAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ResolveLocomotionAnimSnapshot(DeltaSeconds, LocomotionAnimSnapshot);
	ResolveRagdollSnapshot(RagdollSnapshot);
	PublishLocomotionSnapshotToChooser();

	FNotoHeldItemAnimSnapshot ResolvedSnapshot;
	ResolveHeldItemAnimSnapshot(ResolvedSnapshot);
	if (HasVariantChanged(HeldItemAnimSnapshot, ResolvedSnapshot))
	{
		PreviousHeldItemAnimSnapshot = HeldItemAnimSnapshot;
		ResolvedSnapshot.VariantRevision = HeldItemAnimSnapshot.VariantRevision + 1;
		HeldItemAnimSnapshot = ResolvedSnapshot;
		HeldItemAnimTransitionAlpha = 0.0f;
	}
	else
	{
		ResolvedSnapshot.VariantRevision = HeldItemAnimSnapshot.VariantRevision;
		HeldItemAnimSnapshot = ResolvedSnapshot;
		const float BlendTime = FMath::Max(KINDA_SMALL_NUMBER, HeldItemAnimSnapshot.BlendTime);
		HeldItemAnimTransitionAlpha = FMath::Min(1.0f, HeldItemAnimTransitionAlpha + DeltaSeconds / BlendTime);
	}
}

void UNotoCharacterAnimInstance::ResolveRagdollSnapshot(FNotoRagdollSnapshot& OutSnapshot) const
{
	OutSnapshot = {};
	if (const ANotoCharacter* Character = Cast<ANotoCharacter>(TryGetPawnOwner()))
	{
		if (const UNotoRagdollComponent* RagdollComponent = Character->GetRagdollComponent())
		{
			OutSnapshot = RagdollComponent->GetRagdollSnapshot();
		}
	}
}

void UNotoCharacterAnimInstance::PublishLocomotionSnapshotToChooser()
{
	MovementMode = LocomotionAnimSnapshot.MovementMode;
	MovementMode_LastFrame = LocomotionAnimSnapshot.PreviousMovementMode;
	MovementState = LocomotionAnimSnapshot.LocomotionState;
	MovementState_LastFrame = LocomotionAnimSnapshot.PreviousLocomotionState;
	Gait = LocomotionAnimSnapshot.Gait;
	Gait_LastFrame = LocomotionAnimSnapshot.PreviousGait;
	Stance = LocomotionAnimSnapshot.Stance;
	Stance_LastFrame = LocomotionAnimSnapshot.PreviousStance;
	RotationMode = LocomotionAnimSnapshot.RotationMode;
	RotationMode_LastFrame = LocomotionAnimSnapshot.PreviousRotationMode;
}

void UNotoCharacterAnimInstance::ResolveLocomotionAnimSnapshot(const float DeltaSeconds, FNotoLocomotionAnimSnapshot& OutSnapshot) const
{
	const FNotoLocomotionAnimSnapshot PreviousSnapshot = OutSnapshot;
	OutSnapshot = {};

	const ANotoCharacter* Character = Cast<ANotoCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		return;
	}

	OutSnapshot.bHasCharacter = true;
	OutSnapshot.ActorTransform = Character->GetActorTransform();
	OutSnapshot.PreviousActorTransform = PreviousSnapshot.ActorTransform;
	OutSnapshot.bIsCrouched = Character->bIsCrouched;
	OutSnapshot.OrientationIntent = Character->GetActorRotation();
	OutSnapshot.AimingRotation = Character->GetActorRotation();
	OutSnapshot.PreviousMovementMode = PreviousSnapshot.MovementMode;
	OutSnapshot.PreviousLocomotionState = PreviousSnapshot.LocomotionState;
	OutSnapshot.PreviousGait = PreviousSnapshot.Gait;
	OutSnapshot.PreviousStance = PreviousSnapshot.Stance;
	OutSnapshot.PreviousRotationMode = PreviousSnapshot.RotationMode;
	OutSnapshot.Stance = OutSnapshot.bIsCrouched ? ENotoLocomotionStance::Crouch : ENotoLocomotionStance::Stand;
	OutSnapshot.RotationMode = ENotoLocomotionRotationMode::Strafe;

	const UNotoPlayerPawnComponent* PlayerPawnComponent = Character->GetPlayerPawnComponent();
	if (PlayerPawnComponent)
	{
		OutSnapshot.InputState = PlayerPawnComponent->GetLocomotionInputState();
		OutSnapshot.Gait = PlayerPawnComponent->GetResolvedGait();
		OutSnapshot.RotationMode = PlayerPawnComponent->GetResolvedRotationMode();
	}

	const UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	OutSnapshot.Velocity = MovementComponent->Velocity;
	OutSnapshot.PreviousVelocity = PreviousSnapshot.Velocity;
	OutSnapshot.InputAcceleration = MovementComponent->GetCurrentAcceleration();
	OutSnapshot.PreviousInputAcceleration = PreviousSnapshot.InputAcceleration;
	OutSnapshot.MaxAcceleration = MovementComponent->GetMaxAcceleration();
	OutSnapshot.MaxDeceleration = MovementComponent->GetMaxBrakingDeceleration();
	OutSnapshot.AccelerationAmount = OutSnapshot.MaxAcceleration > KINDA_SMALL_NUMBER
		? OutSnapshot.InputAcceleration.Length() / OutSnapshot.MaxAcceleration
		: 0.0f;
	OutSnapshot.bHasAcceleration = OutSnapshot.AccelerationAmount > 0.0f;
	OutSnapshot.Speed2D = OutSnapshot.Velocity.Size2D();
	OutSnapshot.bHasVelocity = OutSnapshot.Speed2D > 5.0f;
	OutSnapshot.bIsMoving = !OutSnapshot.Velocity.IsNearlyZero(0.1f);
	OutSnapshot.LocomotionState = OutSnapshot.bIsMoving ? ENotoLocomotionState::Moving : ENotoLocomotionState::Idle;
	if (const UNotoRagdollComponent* RagdollComponent = Character->GetRagdollComponent(); RagdollComponent && RagdollComponent->IsRagdollActive())
	{
		OutSnapshot.MovementMode = ENotoLocomotionMovementMode::Ragdoll;
		OutSnapshot.LocomotionState = ENotoLocomotionState::Idle;
		OutSnapshot.bIsMoving = false;
	}
	else if (const UNotoTraversalComponent* TraversalComponent = Character->GetTraversalComponent(); TraversalComponent && TraversalComponent->IsTraversalActive())
	{
		OutSnapshot.MovementMode = ENotoLocomotionMovementMode::Traversing;
	}
	else
	{
		switch (MovementComponent->MovementMode)
		{
		case MOVE_Falling:
			OutSnapshot.MovementMode = ENotoLocomotionMovementMode::InAir;
			break;
		case MOVE_Flying:
			OutSnapshot.MovementMode = ENotoLocomotionMovementMode::Flying;
			break;
		default:
			OutSnapshot.MovementMode = ENotoLocomotionMovementMode::OnGround;
			break;
		}
	}
	OutSnapshot.bJustLanded = PreviousSnapshot.MovementMode == ENotoLocomotionMovementMode::InAir
		&& OutSnapshot.MovementMode == ENotoLocomotionMovementMode::OnGround;
	OutSnapshot.LandingVelocity = OutSnapshot.bJustLanded ? PreviousSnapshot.Velocity : FVector::ZeroVector;
	OutSnapshot.VelocityAcceleration = (OutSnapshot.Velocity - OutSnapshot.PreviousVelocity) / FMath::Max(DeltaSeconds, 0.001f);
	OutSnapshot.RelativeAcceleration = OutSnapshot.ActorTransform.GetRotation().UnrotateVector(OutSnapshot.VelocityAcceleration);
	if (!OutSnapshot.InputAcceleration.IsNearlyZero() && !OutSnapshot.VelocityAcceleration.IsNearlyZero())
	{
		const float AccelerationLimit = FVector::DotProduct(OutSnapshot.InputAcceleration, OutSnapshot.Velocity) > 0.0f
			? OutSnapshot.MaxAcceleration
			: OutSnapshot.MaxDeceleration;
		if (AccelerationLimit > KINDA_SMALL_NUMBER)
		{
			const FVector ClampedAcceleration = OutSnapshot.VelocityAcceleration.GetClampedToMaxSize(AccelerationLimit);
			OutSnapshot.RelativeAccelerationAmount = OutSnapshot.ActorTransform.GetRotation().UnrotateVector(ClampedAcceleration / AccelerationLimit);
		}
	}
	OutSnapshot.LastNonZeroVelocity = OutSnapshot.bHasVelocity ? OutSnapshot.Velocity : PreviousSnapshot.LastNonZeroVelocity;
}

void UNotoCharacterAnimInstance::ResolveHeldItemAnimSnapshot(FNotoHeldItemAnimSnapshot& OutSnapshot) const
{
	OutSnapshot = {};

	const ANotoCharacter* Character = Cast<ANotoCharacter>(TryGetPawnOwner());
	const UNotoEquippedItemComponent* EquippedItemComponent = Character ? Character->GetEquippedItemComponent() : nullptr;
	const UNotoHeldItemAnimationProfile* Profile = EquippedItemComponent ? EquippedItemComponent->GetDisplayedHeldItemAnimationProfile() : nullptr;
	if (!Character || !Profile)
	{
		return;
	}

	const UNotoPlayerPawnComponent* PlayerPawnComponent = Character->FindComponentByClass<UNotoPlayerPawnComponent>();
	const ENotoHeldItemPoseMode PoseMode = PlayerPawnComponent ? PlayerPawnComponent->GetHeldItemPoseMode() : ENotoHeldItemPoseMode::HipFire;
	const FNotoHeldItemPoseVariant* Variant = Profile->FindPoseVariant(PoseMode, Character->bIsCrouched);
	if (!Variant)
	{
		return;
	}

	OutSnapshot.bHasHeldItemAnimation = true;
	OutSnapshot.PoseMode = PoseMode;
	OutSnapshot.bCrouched = Character->bIsCrouched;
	OutSnapshot.StanceSequence = Variant->StanceSequence;
	OutSnapshot.bHasAimOffset = IsValid(Variant->AimOffset);
	OutSnapshot.AimOffset = Variant->AimOffset;
	OutSnapshot.BlendTime = Variant->BlendTime;
	OutSnapshot.AimOffsetAlpha = Variant->AimOffsetAlpha;
	OutSnapshot.SupportHandIKAlpha = Profile->GetSupportHandIKAlpha();
	OutSnapshot.DisableLeftHandIKAlpha = FMath::Clamp(GetCurveValue(TEXT("DisableLHandIK")), 0.0f, 1.0f);

	USkeletalMeshComponent* CharacterMesh = GetSkelMeshComponent();
	ANotoEquippedItemActor* EquippedActor = EquippedItemComponent->GetDisplayedActor();
	USkeletalMeshComponent* ItemMesh = EquippedActor ? EquippedActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	const FName SupportSocketName = Profile->GetSupportHandSocketName();
	if (!CharacterMesh || !ItemMesh || SupportSocketName.IsNone() || !ItemMesh->DoesSocketExist(SupportSocketName)
		|| CharacterMesh->GetBoneIndex(TEXT("weapon_r")) == INDEX_NONE)
	{
		return;
	}

	const FTransform WeaponBoneWorld = CharacterMesh->GetSocketTransform(TEXT("weapon_r"), RTS_World);
	const FTransform SupportSocketWorld = ItemMesh->GetSocketTransform(SupportSocketName, RTS_World);
	OutSnapshot.bHasSupportHandTarget = true;
	OutSnapshot.SupportHandTargetWeaponSpace = SupportSocketWorld.GetRelativeTransform(WeaponBoneWorld);
	OutSnapshot.SupportHandTargetComponentSpace = SupportSocketWorld.GetRelativeTransform(CharacterMesh->GetComponentTransform());
}
