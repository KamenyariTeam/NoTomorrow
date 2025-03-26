// © 2025 Kamenyari. All rights reserved.

#include "NotoDeveloperSettings.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Application/SlateApplication.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoDeveloperSettings)

#define LOCTEXT_NAMESPACE "NotoCheats"

UNotoDeveloperSettings::UNotoDeveloperSettings()
{
}

FName UNotoDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

#if WITH_EDITOR
void UNotoDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplySettings();
}

void UNotoDeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);
	ApplySettings();
}

void UNotoDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();
	ApplySettings();
}

void UNotoDeveloperSettings::ApplySettings()
{
}

void UNotoDeveloperSettings::OnPlayInEditorStarted() const
{
	// Show a notification if an experience override is active.
	if (ExperienceOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("ExperienceOverrideActive", "Developer Settings Override\nExperience {0}"),
			FText::FromName(ExperienceOverride.PrimaryAssetName)
		));
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
