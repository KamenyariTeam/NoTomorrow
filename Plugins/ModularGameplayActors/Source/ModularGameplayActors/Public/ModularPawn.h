#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "ModularPawn.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPawn: public APawn
{
	GENERATED_BODY()
public:

	//~Begin Actor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End Actor interface
};
