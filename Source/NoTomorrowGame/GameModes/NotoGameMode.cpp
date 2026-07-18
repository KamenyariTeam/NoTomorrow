// © 2025 Kamenyari. All rights reserved.


#include "NotoGameMode.h"

#include "Character/NotoCharacter.h"
#include "GameFramework/HUD.h"
#include "ModularGameState.h"
#include "Player/NotoPlayerController.h"

ANotoGameMode::ANotoGameMode(const FObjectInitializer& Initializer) : Super(Initializer)
{
	DefaultPawnClass = ANotoCharacter::StaticClass();
	PlayerControllerClass = ANotoPlayerController::StaticClass();
	HUDClass = AHUD::StaticClass();
	GameStateClass = AModularGameStateBase::StaticClass();
}
