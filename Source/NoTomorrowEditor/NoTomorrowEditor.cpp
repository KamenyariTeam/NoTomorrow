#include "NoTomorrowEditor.h"

#include "Editor/UnrealEdEngine.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GameEditorStyle.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "UObject/UObjectIterator.h"
#include "UnrealEdGlobals.h"
#include "Development/NotoDeveloperSettings.h"

class SWidget;

#define LOCTEXT_NAMESPACE "NotoEditor"

DEFINE_LOG_CATEGORY(LogNotoEditor);


static bool HasPlayWorld()
{
	return GEditor->PlayWorld != nullptr;
}

static bool HasNoPlayWorld()
{
	return !HasPlayWorld();
}

static bool HasPlayWorldAndRunning()
{
	return HasPlayWorld() && !GUnrealEd->PlayWorld->bDebugPauseExecution;
}

static void OpenCommonMap_Clicked(const FString MapPath)
{
	if (ensure(MapPath.Len()))
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(MapPath);
	}
}

static bool CanShowCommonMaps()
{
	return HasNoPlayWorld() && !GetDefault<UNotoDeveloperSettings>()->CommonEditorMaps.IsEmpty();
}

static TSharedRef<SWidget> GetCommonMapsDropdown()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	
	for (const FSoftObjectPath& Path : GetDefault<UNotoDeveloperSettings>()->CommonEditorMaps)
	{
		if (!Path.IsValid())
		{
			continue;
		}
		
		const FText DisplayName = FText::FromString(Path.GetAssetName());
		MenuBuilder.AddMenuEntry(
			DisplayName,
			LOCTEXT("CommonPathDescription", "Opens this map in the editor"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&OpenCommonMap_Clicked, Path.ToString()),
				FCanExecuteAction::CreateStatic(&HasNoPlayWorld),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&HasNoPlayWorld)
			)
		);
	}

	return MenuBuilder.MakeWidget();
}

static void RegisterGameEditorMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->AddSection("PlayGameExtensions", TAttribute<FText>(), FToolMenuInsert("Play", EToolMenuInsertType::After));

	// Uncomment this to add a custom toolbar that is displayed during PIE
	// Useful for making easy access to changing game state artificially, adding cheats, etc
	// FToolMenuEntry BlueprintEntry = FToolMenuEntry::InitComboButton(
	// 	"OpenGameMenu",
	// 	FUIAction(
	// 		FExecuteAction(),
	// 		FCanExecuteAction::CreateStatic(&HasPlayWorld),
	// 		FIsActionChecked(),
	// 		FIsActionButtonVisible::CreateStatic(&HasPlayWorld)),
	// 	FOnGetContent::CreateStatic(&YourCustomMenu),
	// 	LOCTEXT("GameOptions_Label", "Game Options"),
	// 	LOCTEXT("GameOptions_ToolTip", "Game Options"),
	// 	FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.OpenLevelBlueprint")
	// );
	// BlueprintEntry.StyleNameOverride = "CalloutToolbar";
	// Section.AddEntry(BlueprintEntry);

	FToolMenuEntry CommonMapEntry = FToolMenuEntry::InitComboButton(
		"CommonMapOptions",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateStatic(&HasNoPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&CanShowCommonMaps)),
		FOnGetContent::CreateStatic(&GetCommonMapsDropdown),
		LOCTEXT("CommonMaps_Label", "Common Maps"),
		LOCTEXT("CommonMaps_ToolTip", "Some commonly desired maps while using the editor"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level")
	);
	CommonMapEntry.StyleNameOverride = "CalloutToolbar";
	Section.AddEntry(CommonMapEntry);
}

/**
 * FNotoEditorModule
 */
class FNoTomorrowEditorModule : public FDefaultGameModuleImpl
{
	typedef FNoTomorrowEditorModule ThisClass;

	virtual void StartupModule() override
	{
		FGameEditorStyle::Initialize();

		if (!IsRunningGame())
		{
			if (FSlateApplication::IsInitialized())
			{
				ToolMenusHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterGameEditorMenus));
			}
		}
	}

	virtual void ShutdownModule() override
	{
		// Undo UToolMenus
		if (UObjectInitialized() && ToolMenusHandle.IsValid())
		{
			UToolMenus::UnRegisterStartupCallback(ToolMenusHandle);
		}

		FModuleManager::Get().OnModulesChanged().RemoveAll(this);
		FGameEditorStyle::Shutdown();
	}
	
	FDelegateHandle ToolMenusHandle;
};

IMPLEMENT_MODULE(FNoTomorrowEditorModule, NoTomorrowEditor)

#undef LOCTEXT_NAMESPACE
