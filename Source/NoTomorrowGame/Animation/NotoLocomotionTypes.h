// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NotoLocomotionTypes.generated.h"

UENUM(BlueprintType)
enum class ENotoLocomotionGait : uint8
{
	Walk,
	Run,
	Sprint
};

UENUM(BlueprintType)
enum class ENotoLocomotionMovementMode : uint8
{
	OnGround,
	InAir,
	Sliding,
	Traversing,
	Ragdoll,
	Flying
};

UENUM(BlueprintType)
enum class ENotoLocomotionStance : uint8
{
	Stand,
	Crouch
};

UENUM(BlueprintType)
enum class ENotoLocomotionRotationMode : uint8
{
	OrientToMovement,
	Strafe,
	Aim
};

UENUM(BlueprintType)
enum class ENotoLocomotionMovementDirection : uint8
{
	F,
	B,
	LL,
	LR,
	RL,
	RR
};

UENUM(BlueprintType)
enum class ENotoLocomotionState : uint8
{
	Idle,
	Moving
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoPlayerInputState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bWantsToSprint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bWantsToWalk = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bWantsToStrafe = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bWantsToAim = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bWantsToCrouch = false;
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoLocomotionAnimSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bHasCharacter = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FNotoPlayerInputState InputState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionMovementMode MovementMode = ENotoLocomotionMovementMode::OnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionMovementMode PreviousMovementMode = ENotoLocomotionMovementMode::OnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionState LocomotionState = ENotoLocomotionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionState PreviousLocomotionState = ENotoLocomotionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionGait Gait = ENotoLocomotionGait::Run;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionGait PreviousGait = ENotoLocomotionGait::Run;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionStance Stance = ENotoLocomotionStance::Stand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionStance PreviousStance = ENotoLocomotionStance::Stand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionRotationMode RotationMode = ENotoLocomotionRotationMode::Strafe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ENotoLocomotionRotationMode PreviousRotationMode = ENotoLocomotionRotationMode::Strafe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bIsCrouched = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bIsMoving = false;

	/** True for the first animation update after transitioning from falling to ground movement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bJustLanded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FTransform ActorTransform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FTransform PreviousActorTransform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector PreviousVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector InputAcceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector PreviousInputAcceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector VelocityAcceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector RelativeAcceleration = FVector::ZeroVector;

	/** Velocity acceleration normalized by the active acceleration or braking limit, in actor space. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector LastNonZeroVelocity = FVector::ZeroVector;

	/** Velocity from the final in-air frame that caused the current landing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FVector LandingVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bHasAcceleration = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bHasVelocity = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float AccelerationAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float Speed2D = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float MaxAcceleration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float MaxDeceleration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FRotator OrientationIntent = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	FRotator AimingRotation = FRotator::ZeroRotator;
};
