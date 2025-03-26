// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NotoCharacterMovementComponent.generated.h"

/**
 * FNotoCharacterGroundInfo
 *
 * Stores information about the ground beneath the character.
 */
USTRUCT(BlueprintType)
struct FNotoCharacterGroundInfo
{
	GENERATED_BODY()

	FNotoCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	// The frame number when the ground information was last updated.
	uint64 LastUpdateFrame;

	// The hit result from the ground trace.
	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	// The distance from the character to the ground.
	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

/**
 * UNotoCharacterMovementComponent
 *
 * Custom movement component that extends UCharacterMovementComponent.
 * Provides ground information and preserves replicated acceleration if needed.
 */
UCLASS(Config = Game)
class NOTOMORROWGAME_API UNotoCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UNotoCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	// Overrides movement simulation to preserve replicated acceleration when applicable.
	virtual void SimulateMovement(float DeltaTime) override;

	// Determines whether the character is allowed to jump. This version omits crouch checks.
	virtual bool CanAttemptJump() const override;

	// Returns the latest ground information; updates if outdated.
	UFUNCTION(BlueprintCallable, Category = "Noto|CharacterMovement")
	const FNotoCharacterGroundInfo& GetGroundInfo();

	// Sets an externally replicated acceleration value.
	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:
	// Initializes the component.
	virtual void InitializeComponent() override;

protected:
	// Cached ground information. Do not modify directly; use GetGroundInfo().
	FNotoCharacterGroundInfo CachedGroundInfo;

	// If true, the component uses an externally replicated acceleration.
	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};
