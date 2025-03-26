#include "GameplayExperience.h"

#include "GameFeatureAction.h"
#include "GameplayExperienceActionSet.h"
#include "GameFeaturesSubsystem.h"

#if WITH_EDITORONLY_DATA
void UGameplayExperience::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action: Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif

TArray<FPrimaryAssetId> UGameplayExperience::GetPrimaryAssets() const
{
	TArray<FPrimaryAssetId> PrimaryAssets;
	PrimaryAssets.Add(GetPrimaryAssetId());

	int32 Index = 0;
	for (const UGameplayExperienceActionSet* ActionSet: ActionSets)
	{
		if (ensureAlwaysMsgf(ActionSet != nullptr, TEXT("Action set [%d] is nullptr for experience [%s]"), Index, *GetPrimaryAssetId().ToString()))
		{
			PrimaryAssets.Add(ActionSet->GetPrimaryAssetId());
		}
		++Index;
	}

	return PrimaryAssets;
}

TArray<FString> UGameplayExperience::GetGameFeaturePluginURLs() const
{
	TArray<FString> PluginURLs;
	auto AddFeaturePluginURL = [&PluginURLs, this](const FString& GameFeatureName)
	{
		FString PluginURL;
		if (UGameFeaturesSubsystem::Get().GetPluginURLByName(GameFeatureName, PluginURL))
		{
			PluginURLs.AddUnique(PluginURL);
		}
		else
		{
			ensureAlwaysMsgf(false, TEXT("Failed to find plugin URL for Game Feature plugin name [%s] for experience [%s]"),
				*GameFeatureName, *GetPrimaryAssetId().ToString());
		}
	};
	
	for (const FString& GameFeatureName: GameFeatures)
	{
		AddFeaturePluginURL(GameFeatureName);
	}

	for (const UGameplayExperienceActionSet* ActionSet: ActionSets)
	{
		for (const FString& GameFeatureName: ActionSet->GameFeatures)
		{
			AddFeaturePluginURL(GameFeatureName);
		}
	}

	return PluginURLs;
}

TArray<UGameFeatureAction*> UGameplayExperience::GetGameFeatureActions() const
{
	TArray<UGameFeatureAction*> GameFeatureActions;
	GameFeatureActions.Append(Actions);

	for (const UGameplayExperienceActionSet* ActionSet: ActionSets)
	{
		GameFeatureActions.Append(ActionSet->Actions);
	}

	return GameFeatureActions;
}
