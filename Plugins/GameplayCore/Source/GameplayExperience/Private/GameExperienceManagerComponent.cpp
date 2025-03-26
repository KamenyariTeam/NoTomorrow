#include "GameExperienceManagerComponent.h"

#include "GameFeatureAction.h"
#include "GameplayExperience.h"
#include "GameFeaturesSubsystem.h"
#include "Engine/AssetManager.h"
#include "GameFeaturePluginOperationResult.h"
#include "GameFeaturesSubsystemSettings.h"

DEFINE_LOG_CATEGORY(LogGameExperience);

UGameExperienceManagerComponent* UGameExperienceManagerComponent::Get(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return Get(World->GetGameState());	
	}
	
	return nullptr;
}

UGameExperienceManagerComponent* UGameExperienceManagerComponent::Get(const AGameStateBase* GameState)
{
	return GameState ? GameState->FindComponentByClass<UGameExperienceManagerComponent>() : nullptr;
}

void UGameExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentExperience != nullptr)
	{
		UnloadCurrentExperience();
		CurrentExperienceState = {};
		CurrentExperience = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void UGameExperienceManagerComponent::LoadGameExperience(FPrimaryAssetId ExperienceId)
{
	if (LoadingExperience)
	{
		check(LoadingExperienceState.IsLoading());
		UE_LOG(LogGameExperience, Error, TEXT("%s: trying to load experience %s while %s is in loading state"),
			*FString(__FUNCTION__), *ExperienceId.ToString(), *LoadingExperience->GetPrimaryAssetId().ToString());
		return;
	}

	if (!ExperienceId.IsValid())
	{
		UE_LOG(LogGameExperience, Error, TEXT("%s: failed to identify experience %s"), *FString(__FUNCTION__), *ExperienceId.ToString());
		return;
	}
	
	const UAssetManager& AssetManager = UAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	
	// @todo: load game experience async?
	UObject* Asset = AssetPath.TryLoad();
#if WITH_EDITOR
	if (Asset == nullptr)
	{
		Asset = FindObject<UObject>(nullptr, *ExperienceId.ToString(), false);
	}
#endif
	
	if (TSubclassOf<UGameplayExperience> GameExperienceClass = Cast<UClass>(Asset))
	{
		Asset = GameExperienceClass->GetDefaultObject();
	}
	check(Asset);
	
	if (Asset != nullptr && CurrentExperience == Asset)
	{
		UE_LOG(LogGameExperience, Error, TEXT("%s: trying to load same experience %s"), *FString(__FUNCTION__), *ExperienceId.ToString());
		return;
	}

	LoadingExperience = CastChecked<UGameplayExperience>(Asset);
	LoadingExperienceState.ExperienceId = ExperienceId;
	check(LoadingExperience);
	
	if (CurrentExperience != nullptr)
	{
		UnloadCurrentExperience();
		CurrentExperienceState = {};
		CurrentExperience = nullptr;
	}
	
	StartLoadingExperience();
}

void UGameExperienceManagerComponent::UnloadCurrentExperience()
{
	check(CurrentExperience != nullptr);
	check(CurrentExperienceState.LoadState == EGameExperienceState::Activated);

	// deactivate game features
	// @todo: if unload was caused by loading another experience, deactivate only game features that are not present in new experience
	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (const FString& PluginURL: CurrentExperienceState.GameFeaturePluginURLs)
	{
		Subsystem.DeactivateGameFeaturePlugin(PluginURL);
	}

	const UWorld* World = GetWorld();
	check(World);
	
	FGameFeatureDeactivatingContext Context(TEXTVIEW("Unknown"), [](FStringView InPauserTag){});
	
	FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World);
	Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);

	// deactivate game feature actions
	TArray<UGameFeatureAction*> GameFeatureActions = CurrentExperience->GetGameFeatureActions();
	for (UGameFeatureAction* GameFeatureAction: GameFeatureActions)
	{
		GameFeatureAction->OnGameFeatureDeactivating(Context);
		GameFeatureAction->OnGameFeatureUnregistering();
	}

	CurrentExperienceState.LoadedGameFeaturesCount = 0;
	CurrentExperienceState.LoadState = EGameExperienceState::Unloaded;

	BroadcastExperienceUnloaded(CurrentExperience);
}

void UGameExperienceManagerComponent::StartLoadingExperience()
{
	check(LoadingExperience != nullptr);
	check(LoadingExperienceState.LoadState == EGameExperienceState::Unloaded);

	// gather primary assets to load
	TArray<FPrimaryAssetId> PrimaryAssetsToLoad = LoadingExperience->GetPrimaryAssets();
	
	LoadingExperienceState.LoadState = EGameExperienceState::LoadingPrimaryAssets;
	UE_LOG(LogGameExperience, Log, TEXT("Started loading primary assets [%d]: Loading Experience [%s]"),
		PrimaryAssetsToLoad.Num(), *LoadingExperienceState.ExperienceId.ToString());

	if (PrimaryAssetsToLoad.Num() == 0)
	{
		// no primary assets to load
		OnPrimaryAssetsLoaded();
		return;
	}

	// Gather assets bundles associated with the experience
	TArray<FName> BundlesToLoad;
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || !IsRunningDedicatedServer();
	const bool bLoadServer = GIsEditor || !IsRunningClientOnly();

	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}
	
	UAssetManager& AssetManager = UAssetManager::Get();
	TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssets(
		PrimaryAssetsToLoad, BundlesToLoad, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority
	);

	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnPrimaryAssetsLoaded);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// Assets were already loaded, call the delegate now
		FStreamableHandle::ExecuteDelegate(Delegate);
	}
	else
	{
		Handle->BindCompleteDelegate(Delegate);
		Handle->BindCancelDelegate(FStreamableDelegate::CreateUObject(this, &ThisClass::OnPrimaryAssetsLoadFailed));
	}
}

void UGameExperienceManagerComponent::OnPrimaryAssetsLoaded()
{
	check(LoadingExperienceState.LoadState == EGameExperienceState::LoadingPrimaryAssets);
	UE_LOG(LogGameExperience, Log, TEXT("Finished loading primary assets: Loading Experience [%s]"), *LoadingExperienceState.ExperienceId.ToString());

	StartLoadingGameFeatures();
}

void UGameExperienceManagerComponent::OnPrimaryAssetsLoadFailed()
{
	check(LoadingExperienceState.LoadState == EGameExperienceState::LoadingPrimaryAssets);
	UE_LOG(LogGameExperience, Error, TEXT("Failed to load primary assets for Loading Experience [%s]"), *LoadingExperienceState.ExperienceId.ToString());

	StartLoadingGameFeatures();
}

void UGameExperienceManagerComponent::OnGameFeatureLoaded(const UE::GameFeatures::FResult& Result)
{
	if (++LoadingExperienceState.LoadedGameFeaturesCount == LoadingExperienceState.GameFeaturePluginURLs.Num())
	{
		FinishLoadingGameFeatures();	
	}
}

void UGameExperienceManagerComponent::FinishLoadingGameFeatures()
{
	check(LoadingExperienceState.LoadState == EGameExperienceState::LoadingGameFeatures);
	UE_LOG(LogGameExperience, Log, TEXT("Finished loading primary assets: Loading Experience [%s]"), *LoadingExperienceState.ExperienceId.ToString());

	LoadingExperienceState.LoadState = EGameExperienceState::Loaded;
	ActivateGameFeatureActions();
}

void UGameExperienceManagerComponent::StartLoadingGameFeatures()
{
	check(LoadingExperience != nullptr);
	check(LoadingExperienceState.LoadState == EGameExperienceState::LoadingPrimaryAssets);
	
	// find URLs for game feature plugins
	LoadingExperienceState.LoadedGameFeaturesCount = 0;
	LoadingExperienceState.GameFeaturePluginURLs = LoadingExperience->GetGameFeaturePluginURLs();
	
	LoadingExperienceState.LoadState = EGameExperienceState::LoadingGameFeatures;
	UE_LOG(LogGameExperience, Log, TEXT("Started loading game features [%d]: Loading Experience [%s]"),
		LoadingExperienceState.GameFeaturePluginURLs.Num(), *LoadingExperienceState.ExperienceId.ToString());
	
	if (LoadingExperienceState.GameFeaturePluginURLs.Num() == 0)
	{
		// no game features to load, finish loading
		FinishLoadingGameFeatures();
		return;
	}

	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (const FString& PluginURL: LoadingExperienceState.GameFeaturePluginURLs)
	{
		Subsystem.LoadGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeatureLoaded));
	}
}

void UGameExperienceManagerComponent::ActivateGameFeatureActions()
{
	check(LoadingExperience != nullptr);
	check(LoadingExperienceState.LoadState == EGameExperienceState::Loaded);
	
	FGameFeatureActivatingContext Context;

	const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);

	// activate game feature actions
	TArray<UGameFeatureAction*> GameFeatureActions  = LoadingExperience->GetGameFeatureActions();
	for (UGameFeatureAction* GameFeatureAction: GameFeatureActions)
	{
		GameFeatureAction->OnGameFeatureRegistering();
		GameFeatureAction->OnGameFeatureLoading();
		GameFeatureAction->OnGameFeatureActivating(Context);
	}
	
	OnGameExperienceLoadCompleted();
}

void UGameExperienceManagerComponent::OnGameExperienceLoadCompleted()
{
	check(LoadingExperience != nullptr);
	check(LoadingExperienceState.LoadState == EGameExperienceState::Loaded);

	// transfer loading state
	LoadingExperienceState.LoadState = EGameExperienceState::Activated;
	CurrentExperienceState = LoadingExperienceState;
	LoadingExperienceState = {};

	// transfer loading experience
	CurrentExperience = LoadingExperience;
	LoadingExperience = nullptr;

	BroadcastExperienceLoaded(CurrentExperience);
}

bool UGameExperienceManagerComponent::IsGameExperienceLoaded() const
{
	return LoadingExperience == nullptr && CurrentExperience != nullptr;
}

bool UGameExperienceManagerComponent::IsGameExperienceLoading() const
{
	return LoadingExperience != nullptr;
}

const UGameplayExperience* UGameExperienceManagerComponent::GetGameExperience() const
{
	if (LoadingExperience)
	{
		return nullptr;
	}
	
	return CurrentExperience;
}

const UGameplayExperience* UGameExperienceManagerComponent::GetGameExperienceChecked() const
{
	check(LoadingExperience == nullptr && CurrentExperience != nullptr);
	return CurrentExperience;
}

FDelegateHandle UGameExperienceManagerComponent::CallOrRegister_OnGameExperienceLoaded_HighPriority(const UObject* WorldContextObject, FGameExperienceEvent::FDelegate Delegate)
{
	UGameExperienceManagerComponent* ExperienceManager = UGameExperienceManagerComponent::Get(WorldContextObject);
	if (ExperienceManager == nullptr)
	{
		Delegate.Execute();
		return {};
	}

	if (ExperienceManager->IsGameExperienceLoaded())
	{
		Delegate.Execute();
		return {};
	}

	return ExperienceManager->ExperienceLoadedEvent_HighPriority.Add(MoveTemp(Delegate));
}

FDelegateHandle UGameExperienceManagerComponent::CallOrRegister_OnGameExperienceLoaded(const UObject* WorldContextObject, FGameExperienceEvent::FDelegate Delegate)
{
	UGameExperienceManagerComponent* ExperienceManager = UGameExperienceManagerComponent::Get(WorldContextObject);
	if (ExperienceManager == nullptr)
	{
		Delegate.Execute();
		return {};
	}

	if (ExperienceManager->IsGameExperienceLoaded())
	{
		Delegate.Execute();
		return {};
	}

	return ExperienceManager->ExperienceLoadedEvent.Add(MoveTemp(Delegate));
}

FDelegateHandle UGameExperienceManagerComponent::CallOrRegister_OnGameExperienceUnloaded(const UObject* WorldContextObject, FGameExperienceEvent::FDelegate Delegate)
{
	UGameExperienceManagerComponent* ExperienceManager = UGameExperienceManagerComponent::Get(WorldContextObject);
	if (ExperienceManager == nullptr)
	{
		Delegate.Execute();
		return {};
	}

	if (!ExperienceManager->IsGameExperienceLoaded())
	{
		// current experience may be or may be not nullptr at this point
		Delegate.Execute();
		return {};
	}

	return ExperienceManager->ExperienceUnloadedEvent.Add(MoveTemp(Delegate));
}

void UGameExperienceManagerComponent::BroadcastExperienceLoaded(const UGameplayExperience* Experience)
{
	ExperienceLoadedEvent_HighPriority.Broadcast();
	ExperienceLoadedEvent_HighPriority.Clear();
	
	ExperienceLoadedEvent.Broadcast();
	ExperienceLoadedEvent.Clear();
}

void UGameExperienceManagerComponent::BroadcastExperienceUnloaded(const UGameplayExperience* Experience)
{
	ExperienceUnloadedEvent.Broadcast();
	ExperienceUnloadedEvent.Clear();
}


