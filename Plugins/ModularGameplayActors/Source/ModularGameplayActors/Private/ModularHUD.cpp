#include "ModularHUD.h"

#include "HUDComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "Components/GameFrameworkComponentManager.h"

void AModularHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularHUD::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	
	Super::BeginPlay();

	PlayerOwner->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::ReceivePlayerPawn);
	if (APawn* NewPawn = PlayerOwner->GetPawn(); IsValid(NewPawn))
	{
		ReceivePlayerPawn(NewPawn);
	}
}

void AModularHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	if (IsValid(PlayerOwner))
	{
		PlayerOwner->GetOnNewPawnNotifier().RemoveAll(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AModularHUD::DrawHUD()
{
	Super::DrawHUD();

	for (TComponentIterator<UHUDComponent> It(this); It; ++It)
	{
		It->DrawHUD(Canvas, DebugCanvas);
	}
}

void AModularHUD::ReceivePlayerPawn(APawn* NewPawn)
{
	for (TComponentIterator<UHUDComponent> It(this); It; ++It)
	{
		It->ReceivePlayerPawn(NewPawn);	
	}
}
