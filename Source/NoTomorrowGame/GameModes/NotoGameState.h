// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayExperienceProvider.h"
#include "ModularGameState.h"
#include "NotoGameState.generated.h"

class UGameExperienceManagerComponent;
/**
 * Tracks the runtime state of the current level.
 * Useful for globally shared world states such as alert status, timer loops, or heist phases.
 * Inherits from AModularGameStateBase to allow runtime component injection.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoGameState : public AModularGameStateBase, public IGameplayExperienceProvider
{
	GENERATED_BODY()

public:
	ANotoGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin AActor interface
	//~End of AActor interface

	//~Begin IGameplayExperienceProvider interface
	virtual FPrimaryAssetId GetGameExperience() const override { return DefaultGameExperience;}
	//~End of IGameplayExperienceProvider interface

protected:
	// Handles loading and managing the current gameplay experience
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGameExperienceManagerComponent> ExperienceManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (Validate, AllowedClass = "NotoGameplayExperience"))
	FPrimaryAssetId DefaultGameExperience;
};
