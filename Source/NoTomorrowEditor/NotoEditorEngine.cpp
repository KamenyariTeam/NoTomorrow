// Copyright Epic Games, Inc. All Rights Reserved.

#include "NotoEditorEngine.h"

#include "Development/NotoDeveloperSettings.h"
#include "Engine/GameInstance.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameModes/NotoWorldSettings.h"
#include "Settings/ContentBrowserSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Widgets/Notifications/SNotificationList.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoEditorEngine)

class IEngineLoop;

#define LOCTEXT_NAMESPACE "NotoEditor"

UNotoEditorEngine::UNotoEditorEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNotoEditorEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

void UNotoEditorEngine::Start()
{
	Super::Start();
}

void UNotoEditorEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	Super::Tick(DeltaSeconds, bIdleMode);
	
	FirstTickSetup();
}

void UNotoEditorEngine::FirstTickSetup()
{
	if (bFirstTickSetup)
	{
		return;
	}

	bFirstTickSetup = true;

	// Force show plugin content on load.
	GetMutableDefault<UContentBrowserSettings>()->SetDisplayPluginFolders(true);

}

FGameInstancePIEResult UNotoEditorEngine::PreCreatePIEInstances(const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32& InNumOnlinePIEInstances)
{
	if (const ANotoWorldSettings* NotoWorldSettings = Cast<ANotoWorldSettings>(EditorWorld->GetWorldSettings()))
	{
		if (NotoWorldSettings->bForceStandaloneNetMode)
		{
			EPlayNetMode OutPlayNetMode;
			PlaySessionRequest->EditorPlaySettings->GetPlayNetMode(OutPlayNetMode);
			if (OutPlayNetMode != PIE_Standalone)
			{
				PlaySessionRequest->EditorPlaySettings->SetPlayNetMode(PIE_Standalone);

				FNotificationInfo Info(LOCTEXT("ForcingStandaloneForFrontend", "Forcing NetMode: Standalone for the Frontend"));
				Info.ExpireDuration = 2.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
	}

	//@TODO: Should add delegates that a *non-editor* module could bind to for PIE start/stop instead of poking directly
	GetDefault<UNotoDeveloperSettings>()->OnPlayInEditorStarted();

	//
	FGameInstancePIEResult Result = Super::PreCreatePIEServerInstance(bAnyBlueprintErrors, bStartInSpectatorMode, PIEStartTime, bSupportsOnlinePIE, InNumOnlinePIEInstances);

	return Result;
}

#undef LOCTEXT_NAMESPACE
