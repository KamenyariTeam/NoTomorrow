// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "InputMappingContext.h"
#include "NotoHeroComponent.generated.h"

class UInputComponent;
class UNotoInputConfig;

/**
 * Component that sets up input and camera handling for player controlled pawns (or bots that simulate players).
 * This depends on a PawnExtensionComponent to coordinate initialization.
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoHeroComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UNotoHeroComponent(const FObjectInitializer& ObjectInitializer);
	
	/** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
	bool IsReadyToBindInputs() const;
	
	/** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
	static const FName NAME_BindInputsNow;

protected:
	//~ Begin UPawnComponent interface
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UPawnComponent interface
	
	void InitializePlayerInput();
	
	// Input binding callbacks
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(/*const FInputActionValue& InputActionValue*/);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UNotoInputConfig> DefaultInputConfig;

	/** True if input bindings have been applied. */
	bool bReadyToBindInputs;
};
