#include "GameplayExperienceProvider.h"

bool IGameplayExperienceProvider::GetGameExperience(const UObject* Provider, FPrimaryAssetId& OutPrimaryAssetId)
{
	if (const IGameplayExperienceProvider* ProviderInterface = Cast<IGameplayExperienceProvider>(Provider))
	{
		OutPrimaryAssetId = ProviderInterface->GetGameExperience();
		return OutPrimaryAssetId.IsValid();
	}

	return false;
}
