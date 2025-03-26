// © 2025 Kamenyari. All rights reserved.


#include "NotoGameState.h"

#include "GameExperienceManagerComponent.h"

ANotoGameState::ANotoGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent = CreateDefaultSubobject<UGameExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}
