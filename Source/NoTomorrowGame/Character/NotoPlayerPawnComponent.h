// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "NotoPlayerPawnComponent.generated.h"

class AController;
class APlayerController;
class UInputComponent;
class UNotoInputConfig;
class UNotoInteractionComponent;
struct FInputActionValue;

/** Authored behavior for one movement state. Add states here instead of adding state-specific movement code. */
USTRUCT(BlueprintType)
struct FNotoMovementStateConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "MovementState"))
	FGameplayTag StateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ClampMin = "0.0", Units = "cm/s"))
	float MaxWalkSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ClampMin = "0.0", Units = "cm"))
	float NoiseRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Noise"))
	FGameplayTag NoiseTag;
};

/** Owns player-specific input binding and cursor aiming for a pawn. */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoPlayerPawnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UNotoPlayerPawnComponent(const FObjectInitializer& ObjectInitializer);

	void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	UFUNCTION(BlueprintCallable, Category = "Noto|Movement")
	void SetMovementState(FGameplayTag NewStateTag);

	UFUNCTION(BlueprintPure, Category = "Noto|Movement")
	FGameplayTag GetMovementState() const { return MovementState; }

	/** Lets a future user-settings system reverse the interaction modifier at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Noto|Input")
	void SetPickupActiveSlotModifierReversed(bool bReversed) { bPickupActiveSlotModifierReversed = bReversed; }

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~End of UActorComponent interface

protected:
	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	void RefreshAimTickEnabled();
	void RefreshMovementNoiseTimer();
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Aim(const FInputActionValue& InputActionValue);
	void Input_ToggleSneak();
	void Input_Interact();
	void Input_InteractModified();
	void Input_Fire();
	void Input_Reload();
	void Input_Drop();
	void TryInteract(bool bModifierHeld);
	void UpdateAimFromMouseCursor();
	bool GetMouseAimDirection(const APlayerController& PlayerController, const APawn& Pawn, FVector& OutAimDirection) const;
	const FNotoMovementStateConfig* FindMovementStateConfig(FGameplayTag StateTag) const;
	void ApplyMovementState();
	void ReportMovementNoise();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Input")
	TObjectPtr<UNotoInputConfig> DefaultInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Movement", Meta = (TitleProperty = "StateTag"))
	TArray<FNotoMovementStateConfig> MovementStates;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Movement")
	FGameplayTag MovementState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Movement|Noise", Meta = (ClampMin = "0.0", Units = "s"))
	float MovementNoiseInterval = 0.25f;

	FTimerHandle MovementNoiseTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Input|Gamepad", Meta = (ClampMin = "0.0", Units = "cm"))
	float GamepadAimRadius = 300.0f;

	/** Default: the interaction modifier preserves the active slot. Reversed: it selects the collected item. */
	bool bPickupActiveSlotModifierReversed = false;
};
