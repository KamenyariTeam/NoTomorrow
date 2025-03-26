// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "InputMappingContext.h"
#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
#include "NotoHeroComponent.generated.h"

class UInputComponent;
class UNotoInputConfig;

UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoHeroComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UNotoHeroComponent(const FObjectInitializer& ObjectInitializer);

	/** Adds additional input configuration (for context-specific inputs). */
	void AddAdditionalInputConfig(const UNotoInputConfig* InputConfig);

	/** Removes an additional input configuration if added. */
	void RemoveAdditionalInputConfig(const UNotoInputConfig* InputConfig);

	/** Returns true if the component is ready for input binding. */
	bool IsReadyToBindInputs() const;

	/** Initializes player input bindings. Call from Pawn's BeginPlay after InputComponent is set up. */
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);

protected:
	// Input binding callbacks
	void Input_Move(const struct FInputActionValue& InputActionValue);
	void Input_LookMouse(const struct FInputActionValue& InputActionValue);
	void Input_LookStick(const struct FInputActionValue& InputActionValue);

	// Default input mappings that can be set in the editor.
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
	
	/** The input configuration asset to use for binding input actions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UNotoInputConfig> InputConfig;

	/** True if input bindings have been applied. */
	bool bReadyToBindInputs;
};
