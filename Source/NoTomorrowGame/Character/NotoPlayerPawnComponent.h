// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "Animation/NotoHeldItemAnimationTypes.h"
#include "Animation/NotoLocomotionTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "NotoPlayerPawnComponent.generated.h"

class AController;
class APlayerController;
class UInputComponent;
class UNotoInputConfig;
class UNotoInteractionComponent;
struct FInputActionValue;

/** Authored speed and noise behavior for one locomotion gait. */
USTRUCT(BlueprintType)
struct FNotoGaitConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ENotoLocomotionGait Gait = ENotoLocomotionGait::Run;

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

	UFUNCTION(BlueprintPure, Category = "Noto|Movement")
	ENotoLocomotionGait GetResolvedGait() const { return ResolvedGait; }

	UFUNCTION(BlueprintPure, Category = "Noto|Movement")
	ENotoLocomotionRotationMode GetResolvedRotationMode() const { return ResolvedRotationMode; }

	UFUNCTION(BlueprintPure, Category = "Noto|Movement")
	bool IsSprintRequested() const { return bSprintRequested; }

	/** Local input intent for animation presentation. It does not grant gameplay state. */
	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Locomotion")
	FNotoPlayerInputState GetLocomotionInputState() const;

	/** Local presentation intent consumed by the character AnimInstance. It does not grant gameplay state. */
	UFUNCTION(BlueprintCallable, Category = "Noto|Animation|Held Item")
	void SetHeldItemPoseMode(ENotoHeldItemPoseMode NewPoseMode) { HeldItemPoseMode = NewPoseMode; }

	UFUNCTION(BlueprintPure, Category = "Noto|Animation|Held Item")
	ENotoHeldItemPoseMode GetHeldItemPoseMode() const { return HeldItemPoseMode; }

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
	void ResolveLocomotionState();
	bool HasLocomotionBlocker() const;
	bool IsSprintAllowed() const;
	bool CanAcceptMovementInput() const;
	bool CanUseAimFacing() const;
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Aim(const FInputActionValue& InputActionValue);
	void Input_Jump();
	void Input_StopJumping();
	void Input_ToggleCrouch();
	void Input_StartSprint();
	void Input_StopSprint();
	void Input_ToggleWalk();
	void Input_Interact();
	void Input_InteractModified();
	void Input_Fire();
	void Input_Reload();
	void Input_Drop();
	void TryInteract(bool bModifierHeld);
	void UpdateAimFromMouseCursor();
	bool GetMouseAimDirection(const APlayerController& PlayerController, const APawn& Pawn, FVector& OutAimDirection) const;
	const FNotoGaitConfig* FindGaitConfig(ENotoLocomotionGait Gait) const;
	void ApplyLocomotionState();
	void ReportMovementNoise();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Input")
	TObjectPtr<UNotoInputConfig> DefaultInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Movement", Meta = (TitleProperty = "Gait"))
	TArray<FNotoGaitConfig> GaitConfigs;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Movement")
	ENotoLocomotionGait ResolvedGait = ENotoLocomotionGait::Run;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Movement")
	ENotoLocomotionRotationMode ResolvedRotationMode = ENotoLocomotionRotationMode::Strafe;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Movement")
	bool bSprintRequested = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Movement")
	bool bWalkRequested = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Noto|Animation|Held Item")
	ENotoHeldItemPoseMode HeldItemPoseMode = ENotoHeldItemPoseMode::HipFire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Movement|Noise", Meta = (ClampMin = "0.0", Units = "s"))
	float MovementNoiseInterval = 0.25f;

	FTimerHandle MovementNoiseTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Input|Gamepad", Meta = (ClampMin = "0.0", Units = "cm"))
	float GamepadAimRadius = 300.0f;

	/** Default: the interaction modifier preserves the active slot. Reversed: it selects the collected item. */
	bool bPickupActiveSlotModifierReversed = false;
};
