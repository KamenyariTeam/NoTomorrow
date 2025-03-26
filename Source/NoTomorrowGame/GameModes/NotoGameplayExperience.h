// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameplayExperience.h"
#include "Engine/DataAsset.h"
#include "NotoGameplayExperience.generated.h"

class UGameplayExperienceActionSet;
class UGameFeatureAction;
class UNotoPawnData;

/**
 * UNotoExperienceDefinition
 *
 * Data asset that defines an experience.
 */
UCLASS(BlueprintType, NotBlueprintable, Const, Meta = (DisplayName = "Noto Experience Definition", ShortTooltip = "Data asset used to define an experience."))
class NOTOMORROWGAME_API UNotoGameplayExperience : public UGameplayExperience
{
	GENERATED_BODY()

public:	
	// The default pawn data asset used for players.
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TObjectPtr<const UNotoPawnData> DefaultPawnData;
};
