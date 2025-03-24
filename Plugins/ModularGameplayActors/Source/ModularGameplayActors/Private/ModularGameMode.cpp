#include "ModularGameMode.h"

#include "ModularGameState.h"
#include "ModularHUD.h"
#include "ModularPawn.h"
#include "ModularPlayerController.h"
#include "ModularPlayerState.h"

AModularGameMode::AModularGameMode(const FObjectInitializer& Initializer): Super(Initializer)
{
	GameStateClass = AModularGameState::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
	HUDClass = AModularHUD::StaticClass();
}

AModularGameModeBase::AModularGameModeBase(const FObjectInitializer& Initializer): Super(Initializer)
{
	GameStateClass = AModularGameStateBase::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
	HUDClass = AModularHUD::StaticClass();
}
