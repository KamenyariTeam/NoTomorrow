// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "NotoPlayerController.generated.h"

/**
 * The base player controller class used by this project.
 * Acts as a mediator between user input and in-world effects.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

protected:
	//~APlayerController interface
	virtual void BeginPlay() override;
	//~End of APlayerController interface
};
