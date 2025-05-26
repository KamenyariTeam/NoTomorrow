// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "ModularCharacter.h"
#include "NotoCharacter.generated.h"

class AController;
class ANotoPlayerController;
class ANotoPlayerState;
class UInputComponent;
class UGameplayCameraComponent;

/**
 * ANotoCharacter
 *
 * A basic character class that handles movement, input, crouching, and death.
 */
UCLASS(Config = Game, BlueprintType)
class NOTOMORROWGAME_API ANotoCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	// Constructor with default object initializer.
	ANotoCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Returns the player controller, if available.
	UFUNCTION(BlueprintCallable, Category = "Noto|Character")
	ANotoPlayerController* GetNotoPlayerController() const;

	// AActor interface overrides.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;

protected:
	// Disables movement and collision (used during death).
	void DisableMovementAndCollision();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;
};
