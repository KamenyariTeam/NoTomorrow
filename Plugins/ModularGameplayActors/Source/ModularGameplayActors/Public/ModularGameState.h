#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "ModularGameState.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameState: public AGameState
{
	GENERATED_BODY()
public:

	//~Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End AActor interface
	
	virtual void HandleMatchHasStarted() override;
};

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameStateBase: public AGameStateBase
{
	GENERATED_BODY()
public:

	//~Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End AActor interface
};
