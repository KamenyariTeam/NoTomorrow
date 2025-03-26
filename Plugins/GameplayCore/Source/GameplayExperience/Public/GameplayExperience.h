#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "GameplayExperience.generated.h"

class UGameFeatureAction;
class UGameplayExperienceActionSet;

GAMEPLAYEXPERIENCE_API DECLARE_LOG_CATEGORY_EXTERN(LogGameExperience, Log, All);

UCLASS(BlueprintType, NotBlueprintable, Const)
class GAMEPLAYEXPERIENCE_API UGameplayExperience: public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

	/** @return primary assets associated with this game experience */
	TArray<FPrimaryAssetId> GetPrimaryAssets() const;
	
	/** @return game feature plugins associated with this game experience */
	TArray<FString> GetGameFeaturePluginURLs() const;
	
	/** @return game feature actions associated with this game experience */
	TArray<UGameFeatureAction*> GetGameFeatureActions() const;

	/** List of game features to enable */
	UPROPERTY(EditAnywhere, Category = "Gameplay", meta = (Validate))
	TArray<FString> GameFeatures;

	/** List of action sets that activate with this experience */
	UPROPERTY(EditAnywhere, Category = "Gameplay", meta = (Validate))
	TArray<TObjectPtr<const UGameplayExperienceActionSet>> ActionSets;

	/** Additional list of actions to activate with this experience */
	UPROPERTY(EditAnywhere, Category = "Gameplay", meta = (Validate, ValidateRecursive))
	TArray<TObjectPtr<UGameFeatureAction>> Actions;
	
};
