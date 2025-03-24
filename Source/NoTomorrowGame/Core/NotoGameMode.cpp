// © 2025 Kamenyari. All rights reserved.


#include "NotoGameMode.h"

#include "NotoCharacter.h"
#include "NotoGameState.h"
#include "NotoHUD.h"
#include "NotoPlayerController.h"

ANotoGameMode::ANotoGameMode()
{
	DefaultPawnClass = ANotoCharacter::StaticClass();
	PlayerControllerClass = ANotoPlayerController::StaticClass();
	HUDClass = ANotoHUD::StaticClass();
	GameStateClass = ANotoGameState::StaticClass();
}
