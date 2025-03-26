#include "GameplayExperienceActionSet.h"

#include "GameFeatureAction.h"

#if WITH_EDITORONLY_DATA
void UGameplayExperienceActionSet::UpdateAssetBundleData()
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
