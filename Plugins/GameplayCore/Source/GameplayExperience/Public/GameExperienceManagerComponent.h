#pragma once

#include "CoreMinimal.h"
#include "GameplayExperience.h"
#include "Components/GameStateComponent.h"

#include "GameExperienceManagerComponent.generated.h"

class AGameState;
class AGameStateBase;
class UGameplayExperience;
namespace UE::GameFeatures { struct FResult; }

DECLARE_MULTICAST_DELEGATE(FGameExperienceEvent);

enum class EGameExperienceState: uint8
{
	Unloaded,
	LoadingPrimaryAssets,
	LoadingGameFeatures,
	Loaded,
	Activated,
};

struct FGameFeatureExperienceState
{
	EGameExperienceState LoadState = EGameExperienceState::Unloaded;
	FPrimaryAssetId ExperienceId;
	TArray<FString> GameFeaturePluginURLs;
	int32 LoadedGameFeaturesCount = 0;

	bool IsLoading() const
	{
		return LoadState != EGameExperienceState::Unloaded;
	}
};

UCLASS()
class GAMEPLAYEXPERIENCE_API UGameExperienceManagerComponent: public UGameStateComponent
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "Components", DisplayName = "Get Game Experience Component", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static UGameExperienceManagerComponent* Get(const UObject* WorldContextObject);
	static UGameExperienceManagerComponent* Get(const AGameStateBase* GameState);

	//~Begin ActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End ActorComponent interface
	
	/**
	 * Starts request to load new experience.
	 *  If other experience is loaded:
	 *  - deactivates game feature actions for current experience
	 *  - start loading new experience
	 *  - perform diff between required game features for current and new experience, load/unload game features
	 *  - mark old experience for unload, activate game features actions for new experience
	 */
	void LoadGameExperience(FPrimaryAssetId ExperienceId);

	/** @return true if currently loading experience */
	bool IsGameExperienceLoading() const;
	/** @return true if current experience is fully loaded */
	bool IsGameExperienceLoaded() const;

	const UGameplayExperience* GetGameExperience() const;
	/** @return fully loaded current experience, asserts otherwise  */
	const UGameplayExperience* GetGameExperienceChecked() const;

	/**
	 * Subscribe to game experience load high priority delegate
	 * Executes delegate immediately if no experience manager found or experience has already been loaded, otherwise returns valid handle
	 */
	static FDelegateHandle CallOrRegister_OnGameExperienceLoaded_HighPriority(const UObject* WorldContextObject, FGameExperienceEvent::FDelegate Delegate);

	/**
	 * Subscribe to game experience load delegate
	 * Executes delegate immediately if no experience manager found or experience has already been loaded, otherwise returns valid handle
	 */
	static FDelegateHandle CallOrRegister_OnGameExperienceLoaded(const UObject* WorldContextObject, FGameExperienceEvent::FDelegate Delegate);

	/**
	 * Subscribe to game experience unload delegate
	 * Executes delegate immediately if no experience manager found or experience has already been loaded, otherwise returns valid handle
	 */
	static FDelegateHandle CallOrRegister_OnGameExperienceUnloaded(const UObject* WorldContextObject, FGameExperienceEvent::FDelegate Delegate);

protected:

	void BroadcastExperienceLoaded(const UGameplayExperience* Experience);
	void BroadcastExperienceUnloaded(const UGameplayExperience* Experience);
	/** Unload current experience */
	void UnloadCurrentExperience();

	/** Loads experience related primary assets */
	void StartLoadingExperience();
	/** Loads game features associated with loading experience */
	void StartLoadingGameFeatures();
	void FinishLoadingGameFeatures();

	/** Activates game feature actions for loading experience */
	void ActivateGameFeatureActions();

	// Callbacks
	void OnPrimaryAssetsLoaded();
	void OnPrimaryAssetsLoadFailed();
	void OnGameFeatureLoaded(const UE::GameFeatures::FResult& Result);
	void OnGameExperienceLoadCompleted();

	/** Called when experience has finished loading, before @ExperienceLoadedEvent */
	FGameExperienceEvent ExperienceLoadedEvent_HighPriority;
	/** Called when experience has finished loading */
	FGameExperienceEvent ExperienceLoadedEvent;
	/** Called when active experience has finished unloading */
	FGameExperienceEvent ExperienceUnloadedEvent;
	
	UPROPERTY(Transient)
	TObjectPtr<const UGameplayExperience> CurrentExperience;
	
	UPROPERTY(Transient)
	TObjectPtr<const UGameplayExperience> LoadingExperience;

	FGameFeatureExperienceState CurrentExperienceState;
	FGameFeatureExperienceState LoadingExperienceState;
};
