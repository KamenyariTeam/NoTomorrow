// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "NotoCharacter.generated.h"

/**
 * The main playable character class.
 * Handles movement, crouching, stealth behavior, and animation logic, etc.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANotoCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
