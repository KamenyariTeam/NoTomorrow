#include "AsyncAction_GameExperienceReady.h"

#include "GameExperienceManagerComponent.h"
#include "GameFeaturesSubsystem.h"
#include "GameFramework/GameStateBase.h"

UAsyncAction_GameExperienceReady* UAsyncAction_GameExperienceReady::WaitForGameExperienceReady(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);

	UAsyncAction_GameExperienceReady* AsyncAction = NewObject<UAsyncAction_GameExperienceReady>(World);
	AsyncAction->World = World;
	AsyncAction->RegisterWithGameInstance(World->GetGameInstance());

	return AsyncAction;
}

void UAsyncAction_GameExperienceReady::Activate()
{
	Super::Activate();

	if (!World.IsValid())
	{
		SetReadyToDestroy();
		return;
	}

	if (AGameStateBase* GameState = World->GetGameState())
	{
		HandleGameStateSet(GameState);
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &ThisClass::HandleGameStateSet);
	}
}

void UAsyncAction_GameExperienceReady::HandleGameStateSet(AGameStateBase* GameState)
{
	check(GameState);
	if (World.IsValid())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}
	
	UGameExperienceManagerComponent::CallOrRegister_OnGameExperienceLoaded(GameState,
		FGameExperienceEvent::FDelegate::CreateUObject(this, &ThisClass::HandleGameExperienceReady));
}

void UAsyncAction_GameExperienceReady::HandleGameExperienceReady()
{
	OnReady.Broadcast();

	SetReadyToDestroy();
}
