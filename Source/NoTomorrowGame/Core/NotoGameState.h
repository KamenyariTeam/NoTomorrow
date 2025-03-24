// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "NotoGameState.generated.h"

/**
 * Tracks the runtime state of the current level.
 * Useful for globally shared world states such as alert status, timer loops, or heist phases.
 * Inherits from AModularGameStateBase to allow runtime component injection.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoGameState : public AModularGameStateBase
{
	GENERATED_BODY()
};
