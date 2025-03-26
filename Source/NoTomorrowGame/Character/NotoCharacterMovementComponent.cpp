// © 2025 Kamenyari. All rights reserved.

#include "NotoCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoCharacterMovementComponent)

// Console variable for ground trace distance.
namespace NotoCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(
		TEXT("NotoCharacter.GroundTraceDistance"), 
		GroundTraceDistance, 
		TEXT("Distance to trace down when updating ground information."), 
		ECVF_Cheat
	);
};

UNotoCharacterMovementComponent::UNotoCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNotoCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve the externally replicated acceleration.
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}

bool UNotoCharacterMovementComponent::CanAttemptJump() const
{
	// Allow jump if jumping is permitted and the character is on the ground or falling.
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling());
}

void UNotoCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

const FNotoCharacterGroundInfo& UNotoCharacterMovementComponent::GetGroundInfo()
{
	// Return cached info if no update is needed.
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		// When walking, use the current floor information.
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		// For other movement modes, perform a downward trace to find the ground.
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, TraceStart.Z - (NotoCharacter::GroundTraceDistance + CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NotoCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = NotoCharacter::GroundTraceDistance;

		// If using navigation-based walking, ground distance is zero.
		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	// Record the frame of the update.
	CachedGroundInfo.LastUpdateFrame = GFrameCounter;
	return CachedGroundInfo;
}

void UNotoCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

FRotator UNotoCharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	// Return the standard delta rotation.
	return Super::GetDeltaRotation(DeltaTime);
}

float UNotoCharacterMovementComponent::GetMaxSpeed() const
{
	// Return the standard maximum speed.
	return Super::GetMaxSpeed();
}
