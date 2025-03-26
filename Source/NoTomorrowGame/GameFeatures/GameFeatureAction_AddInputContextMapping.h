// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "GameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "GameFeatureAction_AddInputContextMapping.generated.h"

class UInputMappingContext;
class ULocalPlayer;
class UGameInstance;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * Struct used to define an input mapping context asset along with its priority.
 */
USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// Higher priority mappings override lower priority ones.
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;
	
	/** If true, this mapping context is registered with the user settings. */
	UPROPERTY(EditAnywhere, Category="Input")
	bool bRegisterWithSettings = true;
};

/**
 * Game feature action that adds input mapping contexts to the Enhanced Input subsystem
 * for local players. This version is simplified for a singleplayer top-down game.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Mapping"))
class UGameFeatureAction_AddInputContextMapping final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureRegistering() override;
	virtual void OnGameFeatureUnregistering() override;
	//~End of UGameFeatureAction interface

	/** Array of input mapping contexts (and priorities) to be registered. */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<FInputMappingContextAndPriority> InputMappings;

private:
	/** Registers input mapping contexts for all local players in all game instances. */
	void RegisterInputMappingContexts();
	
	/** For a given GameInstance, registers input mappings for each local player. */
	void RegisterInputMappingContextsForGameInstance(UGameInstance* GameInstance);
	
	/** Registers input mappings for a specific LocalPlayer. */
	void RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	/** Unregisters input mapping contexts from all local players. */
	void UnregisterInputMappingContexts();
	
	/** For a given GameInstance, unregisters input mappings from each local player. */
	void UnregisterInputMappingContextsForGameInstance(UGameInstance* GameInstance);
	
	/** Unregisters input mapping contexts for a specific LocalPlayer. */
	void UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);
};
