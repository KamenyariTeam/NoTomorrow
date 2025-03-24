#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "ModularAIController.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularAIController: public AAIController
{
	GENERATED_BODY()
public:

	//~Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End AActor interface
	
};
