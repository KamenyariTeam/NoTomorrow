// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPath.h"
#include "NotoDeveloperSettings.generated.h"

struct FPropertyChangedEvent;

/**
 * Enum for when a cheat should be executed.
 */
UENUM()
enum class ECheatExecutionTime
{
	// When the cheat manager is created.
	OnCheatManagerCreated,

	// When a pawn is possessed by a player.
	OnPlayerPawnPossession
};

/**
 * Structure that defines a cheat command to run.
 */
USTRUCT()
struct FNotoCheatToRun
{
	GENERATED_BODY()

	// When to execute the cheat.
	UPROPERTY(EditAnywhere)
	ECheatExecutionTime Phase = ECheatExecutionTime::OnPlayerPawnPossession;

	// The cheat command.
	UPROPERTY(EditAnywhere)
	FString Cheat;
};

/**
 * Developer settings and editor cheats.
 */
UCLASS(config = EditorPerProjectUserSettings, MinimalAPI)
class UNotoDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UNotoDeveloperSettings();

	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

public:
	// The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Noto, meta = (AllowedTypes = "GameExperience"))
	FPrimaryAssetId ExperienceOverride;

	// Whether to simulate full game flow in PIE.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Noto)
	bool bTestFullGameFlowInPIE = false;

	/**
	 * Determines if force feedback should be played even when the last input device was not a gamepad.
	 */
	UPROPERTY(config, EditAnywhere, Category =Noto, meta = (ConsoleVariable = "NotoPC.ShouldAlwaysPlayForceFeedback"))
	bool bShouldAlwaysPlayForceFeedback = false;

	// Whether to skip loading cosmetic backgrounds in PIE for faster iteration.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Noto)
	bool bSkipLoadingCosmeticBackgroundsInPIE = false;

	// List of cheats to run automatically during Play in Editor.
	UPROPERTY(config, EditAnywhere, Category=Noto)
	TArray<FNotoCheatToRun> CheatsToRun;

#if WITH_EDITORONLY_DATA
	// A list of common maps for the editor toolbar.
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> CommonEditorMaps;
#endif
	
#if WITH_EDITOR
public:
	// Called by the editor engine when Play in Editor starts to show a notification if cheats are active.
	NOTOMORROWGAME_API void OnPlayInEditorStarted() const;

private:
	// Applies any settings changes.
	void ApplySettings();
#endif

public:
	//~UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
#endif
	//~End of UObject interface
};
