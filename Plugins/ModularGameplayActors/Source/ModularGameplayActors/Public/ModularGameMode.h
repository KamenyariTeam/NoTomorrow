#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "ModularGameMode.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameMode: public AGameMode
{
	GENERATED_BODY()
public:
	AModularGameMode(const FObjectInitializer& Initializer);
	
};

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameModeBase: public AGameModeBase
{
	GENERATED_BODY()
public:
	AModularGameModeBase(const FObjectInitializer& Initializer);
	
};

