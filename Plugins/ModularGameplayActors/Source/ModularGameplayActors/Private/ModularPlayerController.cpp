#include "ModularPlayerController.h"

#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "Components/GameFrameworkComponentManager.h"

AModularPlayerController::AModularPlayerController(const FObjectInitializer& Initializer): Super(Initializer)
{
}

void AModularPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	
	Super::EndPlay(EndPlayReason);
}

void AModularPlayerController::ReceivedPlayer()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	
	Super::ReceivedPlayer();
	
	for (TComponentIterator<UControllerComponent> It(this); It; ++It)
	{
		It->ReceivedPlayer();
	}
}

void AModularPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	for (TComponentIterator<UControllerComponent> It(this); It; ++It)
	{
		It->PlayerTick(DeltaTime);
	}
}
