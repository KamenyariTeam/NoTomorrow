// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "NotoGameMode.generated.h"

/**
 * Primary game mode for No Tomorrow.
 */
UCLASS()
class NOTOMORROWGAME_API ANotoGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	ANotoGameMode(const FObjectInitializer& Initializer);
	
	//~AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	//~End of AGameModeBase interface

protected:
	bool IsExperienceLoaded() const;
};
