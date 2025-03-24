// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "NotoPlayerController.generated.h"

/**
 * Handles player input, HUD interaction, and tracing for interactables.
 * Acts as a mediator between user input and in-world effects.
 * Supports Enhanced Input and manages input mapping contexts.
 * Inherits from AModularPlayerController to support feature-based input injection.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoPlayerController : public AModularPlayerController
{
	GENERATED_BODY()
};
