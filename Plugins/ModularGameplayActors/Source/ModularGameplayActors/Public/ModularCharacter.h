#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "ModularCharacter.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularCharacter: public ACharacter
{
	GENERATED_BODY()
public:
	
	//~Begin Actor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End Actor interface
	
};
