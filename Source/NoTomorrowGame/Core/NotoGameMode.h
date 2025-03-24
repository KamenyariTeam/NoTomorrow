// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "NotoGameMode.generated.h"

/**
 * Primary game mode for No Tomorrow.
 * Sets the default pawn, controller, and HUD classes.
 * Manages level-specific rules such as objective triggers and fail conditions.
 * Inherits from AModularGameModeBase to support modular gameplay features.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoGameMode : public AModularGameModeBase
{
	GENERATED_BODY()
};
