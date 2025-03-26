#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "GameplayExperienceActionSet.generated.h"

class UGameFeatureAction;

UCLASS(BlueprintType, NotBlueprintable)
class GAMEPLAYEXPERIENCE_API UGameplayExperienceActionSet: public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

	/** List of actions to activate when action set is activated */
	UPROPERTY(EditAnywhere, Instanced, Category = "Actions", meta = (Validate, ValidateRecursive))
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	/** List of game features to enable for this action set */
	UPROPERTY(EditAnywhere, Category = "Game Feature Dependencies", meta = (Validate))
	TArray<FString> GameFeatures;
	
};
