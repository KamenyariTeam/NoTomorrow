#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "GameplayExperienceProvider.generated.h"

UINTERFACE(BlueprintType)
class GAMEPLAYEXPERIENCE_API UGameplayExperienceProvider: public UInterface
{
	GENERATED_BODY()
};

class GAMEPLAYEXPERIENCE_API IGameplayExperienceProvider
{
	GENERATED_BODY()

public:
	
	static bool GetGameExperience(const UObject* Provider, FPrimaryAssetId& OutPrimaryAssetId);

	virtual FPrimaryAssetId GetGameExperience() const
	{
		return FPrimaryAssetId{};
	}
};
