// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "ModularCharacter.h"
#include "NotoCharacter.generated.h"

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

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;
};
