// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "NotoPlayerController.generated.h"

enum class ECommonInputType : uint8;
class UUserWidget;

/**
 * The base player controller class used by this project.
 * Acts as a mediator between user input and in-world effects.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	ANotoPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Noto|UI")
	void SetGameplayReticleClass(TSubclassOf<UUserWidget> InReticleClass);

	UFUNCTION(BlueprintCallable, Category = "Noto|UI")
	void SetGameplayReticleVisible(bool bVisible);

	void SetGameplayReticlePosition(const FVector2D& ScreenPosition);

	UFUNCTION(BlueprintPure, Category = "Noto|Input")
	bool IsUsingGamepad() const;

protected:
	//~APlayerController interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool SetPause(bool bPause, FCanUnpause CanUnpauseDelegate = FCanUnpause()) override;
	//~End of APlayerController interface

private:
	void HandleInputMethodChanged(ECommonInputType NewInputType);
	void ApplyCursorState();
	void EnsureGameplayReticle();

	UPROPERTY(EditDefaultsOnly, Category = "Noto|UI")
	TSubclassOf<UUserWidget> DefaultGameplayReticleClass;

	UPROPERTY(Transient)
	TSubclassOf<UUserWidget> GameplayReticleClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> GameplayReticleWidget;

	FVector2D GameplayReticlePosition = FVector2D::ZeroVector;
	bool bGameplayReticleVisible = true;
	bool bHasGameplayReticlePosition = false;
};
