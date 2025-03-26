// © 2025 Kamenyari. All rights reserved.

#include "GameFeatureAction_AddInputContextMapping.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();
	RegisterInputMappingContexts();
}

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();
	UnregisterInputMappingContexts();
}

void UGameFeatureAction_AddInputContextMapping::RegisterInputMappingContexts()
{
	// Iterate over all active world contexts.
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& WorldContext : WorldContexts)
	{
		if (UGameInstance* GameInstance = WorldContext.OwningGameInstance)
		{
			RegisterInputMappingContextsForGameInstance(GameInstance);
		}
	}
}

void UGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForGameInstance(UGameInstance* GameInstance)
{
	if (!GameInstance)
	{
		return;
	}
	
	// For each local player in the GameInstance, register input mappings.
	for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
	{
		RegisterInputMappingContextsForLocalPlayer(LocalPlayer);
	}
}

void UGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		for (const FInputMappingContextAndPriority& Entry : InputMappings)
		{
			if (Entry.bRegisterWithSettings)
			{
				// Load the mapping context synchronously.
				if (UInputMappingContext* IMC = Entry.InputMapping.LoadSynchronous())
				{
					InputSubsystem->AddMappingContext(IMC, Entry.Priority);
				}
			}
		}
	}
}

void UGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContexts()
{
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& WorldContext : WorldContexts)
	{
		if (UGameInstance* GameInstance = WorldContext.OwningGameInstance)
		{
			UnregisterInputMappingContextsForGameInstance(GameInstance);
		}
	}
}

void UGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForGameInstance(UGameInstance* GameInstance)
{
	if (!GameInstance)
	{
		return;
	}
	
	for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
	{
		UnregisterInputMappingContextsForLocalPlayer(LocalPlayer);
	}
}

void UGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		for (const FInputMappingContextAndPriority& Entry : InputMappings)
		{
			if (Entry.bRegisterWithSettings)
			{
				if (UInputMappingContext* IMC = Entry.InputMapping.Get())
				{
					InputSubsystem->RemoveMappingContext(IMC);
				}
			}
		}
	}
}
