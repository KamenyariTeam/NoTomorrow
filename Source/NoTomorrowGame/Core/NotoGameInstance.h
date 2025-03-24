// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NotoGameInstance.generated.h"

/**
 * Persistent global context that exists across all levels.
 * Stores session-wide data such as save files, anomaly status, and system managers.
 * Also acts as the entry point for activating GameFeature plugins and runtime systems.
 */
UCLASS()
class NOTOMORROWGAME_API UNotoGameInstance : public UGameInstance
{
	GENERATED_BODY()
};
