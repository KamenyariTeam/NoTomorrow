#include "ModularPlayerState.h"

#include "Components/GameFrameworkComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"

void AModularPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	
	Super::BeginPlay();
}

void AModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	
	Super::EndPlay(EndPlayReason);
}

void AModularPlayerState::Reset()
{
	Super::Reset();

	for (TComponentIterator<UPlayerStateComponent> It(this); It; ++It)
	{
		It->Reset();
	}
}

void AModularPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	for (TComponentIterator<UPlayerStateComponent> It(this); It; ++It)
	{
		UObject* TargetObj = static_cast<UObject*>(FindObjectWithOuter(PlayerState, It->GetClass(), It->GetFName()));
		if (UPlayerStateComponent* TargetComp = Cast<UPlayerStateComponent>(TargetObj))
		{
			It->CopyProperties(TargetComp);
		}
	}
}
