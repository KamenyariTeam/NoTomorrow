// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NotoRagdollTypes.generated.h"

UENUM(BlueprintType)
enum class ENotoRagdollInjuryState : uint8
{
	None,
	Limp,
	Stunned,
	HeadFace,
	HeadBack,
	BodyFront,
	Groin
};

UENUM(BlueprintType)
enum class ENotoRagdollRecoveryOrientation : uint8
{
	FaceUp,
	FaceDown,
	LeftSide,
	RightSide
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoRagdollSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	bool bIsRagdoll = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	float TimeSinceImpact = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	float RollAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	bool bCanRecover = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	ENotoRagdollRecoveryOrientation RecoveryOrientation = ENotoRagdollRecoveryOrientation::FaceUp;

	/** Dot product of the configured body-facing axis against world up. Positive means face-up. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	float FacingUpAmount = 0.0f;

	/** Ground-plane facing direction to apply before authored recovery presentation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	FVector RecoveryFacingDirection = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	FVector2D ImpactDirection = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	ENotoRagdollInjuryState InjuryState = ENotoRagdollInjuryState::None;
};
