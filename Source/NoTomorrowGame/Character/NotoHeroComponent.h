// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "InputMappingContext.h"
#include "NotoHeroComponent.generated.h"

class UInputComponent;
class UNotoInputConfig;
class APlayerController;
class AController;

/**
 * Component that sets up input and camera handling for a locally controlled pawn.
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UPawnComponent interface
	
	void TryInitializePlayerInput();
	void ResetPlayerInputBindings();

	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void HandlePawnRestarted(APawn* Pawn);
	
	// Input binding callbacks
	void Input_Move(const FInputActionValue& InputActionValue);

	/** Updates aiming from the visible cursor's absolute viewport position. */
	void UpdateAimFromMouseCursor();
	bool GetMouseAimDirection(const APlayerController& PlayerController, const APawn& Pawn, FVector& OutAimDirection) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UNotoInputConfig> DefaultInputConfig;

	/** Enable cursor aiming. Disable this when another device (for example, a gamepad) owns aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Mouse")
	bool bAimWithMouseCursor = true;

	/** True if input bindings have been applied. */
	bool bReadyToBindInputs;

	TWeakObjectPtr<APlayerController> BoundPlayerController;
	TWeakObjectPtr<UInputComponent> BoundInputComponent;
};
