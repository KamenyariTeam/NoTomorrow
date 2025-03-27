// © 2025 Kamenyari. All rights reserved.


#include "NotoGameMode.h"

#include "GameExperienceManagerComponent.h"
#include "NotoGameState.h"
#include "Character/NotoCharacter.h"
#include "Player/NotoPlayerController.h"
#include "UI/NotoHUD.h"

ANotoGameMode::ANotoGameMode(const FObjectInitializer& Initializer) : Super(Initializer)
{
	DefaultPawnClass = ANotoCharacter::StaticClass();
	PlayerControllerClass = ANotoPlayerController::StaticClass();
	HUDClass = ANotoHUD::StaticClass();
	GameStateClass = ANotoGameState::StaticClass();
}

void ANotoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

bool ANotoGameMode::IsExperienceLoaded() const
{
	check(GameState);
	UGameExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGameExperienceManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsGameExperienceLoaded();
}
