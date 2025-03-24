#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ModularActor.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularActor: public AActor
{
	GENERATED_BODY()
public:

	//~Begin Actor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End Actor interface
	
};
