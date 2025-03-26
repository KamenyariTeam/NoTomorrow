#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameplayExperience.h"
#include "GameplayExperienceProvider.h"
#include "GameFramework/GameState.h"

#include "GameplayExperienceTests.generated.h"

class UGameExperienceManagerComponent;
class UGameplayExperience;

UCLASS(NotBlueprintable, HideDropdown)
class UGameplayExperienceTestGameFeatureAction: public UGameFeatureAction
{
	GENERATED_BODY()
public:

	virtual void OnGameFeatureRegistering() override
	{
		bRegistered = true;
	}
	virtual void OnGameFeatureUnregistering() override
	{
		bRegistered = false;
	}
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override
	{
		ActivatingContext = Context;
		bActivated = true;
	}
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override
	{
		if (Context == ActivatingContext)
		{
			ActivatingContext = {};
			bActivated = false;
		}
	}

	FORCEINLINE bool IsRegistered() const
	{
		return bRegistered;
	}
	
	FORCEINLINE bool IsActivated() const
	{
		return bActivated;
	}

protected:

	FGameFeatureStateChangeContext ActivatingContext;
	bool bRegistered = false;
	bool bActivated = false;
};

UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class UTestGameplayExperience: public UGameplayExperience
{
	GENERATED_BODY()
public:
	UTestGameplayExperience();
};

UCLASS(NotBlueprintType, NotBlueprintable, HideDropdown)
class AGameplayExperienceTestGameState: public AGameStateBase, public IGameplayExperienceProvider
{
	GENERATED_BODY()
public:
	
	AGameplayExperienceTestGameState();
	
	virtual FPrimaryAssetId GetGameExperience() const override
	{
		return ExperienceID;
	}

	FPrimaryAssetId ExperienceID;

	UPROPERTY()
	TObjectPtr<UGameExperienceManagerComponent> ExperienceManager;
};

