// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "NotoPlayerPawnComponent.generated.h"

class AController;
class APlayerController;
class UInputComponent;
class UNotoInputConfig;
struct FInputActionValue;

/** Owns player-specific input binding and cursor aiming for a pawn. */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoPlayerPawnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UNotoPlayerPawnComponent(const FObjectInitializer& ObjectInitializer);

	void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~End of UActorComponent interface

protected:

	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	void RefreshTickEnabled();
	void Input_Move(const FInputActionValue& InputActionValue);
	void UpdateAimFromMouseCursor();
	bool GetMouseAimDirection(const APlayerController& PlayerController, const APawn& Pawn, FVector& OutAimDirection) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Input")
	TObjectPtr<UNotoInputConfig> DefaultInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Input|Mouse")
	bool bAimWithMouseCursor = true;
};
