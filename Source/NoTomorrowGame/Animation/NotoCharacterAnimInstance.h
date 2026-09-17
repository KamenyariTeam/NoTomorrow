// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/NotoHeldItemAnimationTypes.h"
#include "Animation/NotoLocomotionTypes.h"
#include "Ragdoll/NotoRagdollTypes.h"
#include "NotoCharacterAnimInstance.generated.h"

UCLASS(Blueprintable, Transient)
class NOTOMORROWGAME_API UNotoCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item", Meta = (BlueprintThreadSafe))
	const FNotoHeldItemAnimSnapshot& GetHeldItemAnimSnapshot() const { return HeldItemAnimSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item", Meta = (BlueprintThreadSafe))
	const FNotoHeldItemAnimSnapshot& GetPreviousHeldItemAnimSnapshot() const { return PreviousHeldItemAnimSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item", Meta = (BlueprintThreadSafe))
	float GetHeldItemAnimTransitionAlpha() const { return HeldItemAnimTransitionAlpha; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion", Meta = (BlueprintThreadSafe))
	const FNotoLocomotionAnimSnapshot& GetLocomotionAnimSnapshot() const { return LocomotionAnimSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion", Meta = (BlueprintThreadSafe))
	bool IsLocomotionMoving() const { return LocomotionAnimSnapshot.bIsMoving; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion", Meta = (BlueprintThreadSafe))
	bool HasLocomotionJustLanded() const { return LocomotionAnimSnapshot.bJustLanded; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion", Meta = (BlueprintThreadSafe))
	FVector GetLocomotionLandingVelocity() const { return LocomotionAnimSnapshot.LandingVelocity; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion", Meta = (BlueprintThreadSafe))
	FVector GetLocomotionRelativeAccelerationAmount() const { return LocomotionAnimSnapshot.RelativeAccelerationAmount; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Ragdoll", Meta = (BlueprintThreadSafe))
	const FNotoRagdollSnapshot& GetRagdollSnapshot() const { return RagdollSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion", Meta = (BlueprintThreadSafe))
	ENotoLocomotionGait GetLocomotionGait() const { return LocomotionAnimSnapshot.Gait; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Traversal", Meta = (BlueprintThreadSafe))
	FTransform GetTraversalInteractionTransform() const { return TraversalInteractionTransform; }

	void SetTraversalInteractionTransform(const FTransform& NewTransform) { TraversalInteractionTransform = NewTransform; }

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	void ResolveHeldItemAnimSnapshot(FNotoHeldItemAnimSnapshot& OutSnapshot) const;
	void ResolveLocomotionAnimSnapshot(float DeltaSeconds, FNotoLocomotionAnimSnapshot& OutSnapshot) const;
	void PublishLocomotionSnapshotToChooser();
	void ResolveRagdollSnapshot(FNotoRagdollSnapshot& OutSnapshot) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion")
	FNotoLocomotionAnimSnapshot LocomotionAnimSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionMovementMode MovementMode = ENotoLocomotionMovementMode::OnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionMovementMode MovementMode_LastFrame = ENotoLocomotionMovementMode::OnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionState MovementState = ENotoLocomotionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionState MovementState_LastFrame = ENotoLocomotionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionGait Gait = ENotoLocomotionGait::Run;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionGait Gait_LastFrame = ENotoLocomotionGait::Run;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionStance Stance = ENotoLocomotionStance::Stand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionStance Stance_LastFrame = ENotoLocomotionStance::Stand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionRotationMode RotationMode = ENotoLocomotionRotationMode::Strafe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Locomotion|Chooser")
	ENotoLocomotionRotationMode RotationMode_LastFrame = ENotoLocomotionRotationMode::Strafe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Traversal")
	FTransform TraversalInteractionTransform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Ragdoll")
	FNotoRagdollSnapshot RagdollSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	FNotoHeldItemAnimSnapshot HeldItemAnimSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	FNotoHeldItemAnimSnapshot PreviousHeldItemAnimSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	float HeldItemAnimTransitionAlpha = 1.0f;
};
