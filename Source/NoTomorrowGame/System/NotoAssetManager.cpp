﻿// © 2025 Kamenyari. All rights reserved.


#include "NotoAssetManager.h"

#include "Development/NotoLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoAssetManager)

//////////////////////////////////////////////////////////////////////

static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("Noto.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(UNotoAssetManager::DumpLoadedAssets)
);

//////////////////////////////////////////////////////////////////////

#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FNotoAssetManagerStartupJob(#JobFunc, [this](const FNotoAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}, JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)

//////////////////////////////////////////////////////////////////////

UNotoAssetManager::UNotoAssetManager()
{
	
}

UNotoAssetManager& UNotoAssetManager::Get()
{
	check(GEngine);

	if (UNotoAssetManager* Singleton = Cast<UNotoAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogNoto, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to NotoAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<UNotoAssetManager>();
}

UObject* UNotoAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;

		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		if (IsInitialized())
		{
			return GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if asset manager isn't ready yet.
		return AssetPath.TryLoad();
	}

	return nullptr;
}

bool UNotoAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void UNotoAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

void UNotoAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogNoto, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogNoto, Log, TEXT("  %s"), *GetNameSafe(LoadedAsset));
	}

	UE_LOG(LogNoto, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogNoto, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
}

void UNotoAssetManager::StartInitialLoading()
{
	SCOPED_BOOT_TIMING("UNotoAssetManager::StartInitialLoading");

	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	// Run all the queued up startup jobs
	// DoAllStartupJobs();
}

UPrimaryDataAsset* UNotoAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("NotoEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif
		UE_LOG(LogNoto, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

		// This can be called recursively in the editor because it is called on demand from PostLoad so force a sync load for primary asset and async load the rest in that case
		if (GIsEditor)
		{
			Asset = DataClassPath.LoadSynchronous();
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
			if (Handle.IsValid())
			{
				Handle->WaitUntilComplete(0.0f, false);

				// This should always work
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		// It is not acceptable to fail to load any GameData asset. It will result in soft failures that are hard to diagnose.
		UE_LOG(LogNoto, Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}

// void UNotoAssetManager::DoAllStartupJobs()
// {
// 	SCOPED_BOOT_TIMING("UNotoAssetManager::DoAllStartupJobs");
// 	const double AllStartupJobsStartTime = FPlatformTime::Seconds();
//
// 	if (IsRunningDedicatedServer())
// 	{
// 		// No need for periodic progress updates, just run the jobs
// 		for (const FNotoAssetManagerStartupJob& StartupJob : StartupJobs)
// 		{
// 			StartupJob.DoJob();
// 		}
// 	}
// 	else
// 	{
// 		if (StartupJobs.Num() > 0)
// 		{
// 			float TotalJobValue = 0.0f;
// 			for (const FNotoAssetManagerStartupJob& StartupJob : StartupJobs)
// 			{
// 				TotalJobValue += StartupJob.JobWeight;
// 			}
//
// 			float AccumulatedJobValue = 0.0f;
// 			for (FNotoAssetManagerStartupJob& StartupJob : StartupJobs)
// 			{
// 				const float JobValue = StartupJob.JobWeight;
// 				StartupJob.SubstepProgressDelegate.BindLambda([This = this, AccumulatedJobValue, JobValue, TotalJobValue](float NewProgress)
// 					{
// 						const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;
// 						const float OverallPercentWithSubstep = (AccumulatedJobValue + SubstepAdjustment) / TotalJobValue;
//
// 						This->UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
// 					});
//
// 				StartupJob.DoJob();
//
// 				StartupJob.SubstepProgressDelegate.Unbind();
//
// 				AccumulatedJobValue += JobValue;
//
// 				UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
// 			}
// 		}
// 		else
// 		{
// 			UpdateInitialGameContentLoadPercent(1.0f);
// 		}
// 	}
//
// 	StartupJobs.Empty();
//
// 	UE_LOG(LogNoto, Display, TEXT("All startup jobs took %.2f seconds to complete"), FPlatformTime::Seconds() - AllStartupJobsStartTime);
// }

void UNotoAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	// Could route this to the early startup loading screen
}

#if WITH_EDITOR
void UNotoAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	{
		FScopedSlowTask SlowTask(0, NSLOCTEXT("NotoEditor", "BeginLoadingPIEData", "Loading PIE Data"));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);

		// Intentionally after GetGameData to avoid counting GameData time in this timer
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE asset preloading complete"), nullptr);

		// You could add preloading of anything else needed for the experience we'll be using here
		// (e.g., by grabbing the default experience from the world settings + the experience override in developer settings)
	}
}
#endif
