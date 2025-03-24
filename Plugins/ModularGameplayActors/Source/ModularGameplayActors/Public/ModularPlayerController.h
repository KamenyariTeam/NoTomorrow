#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "ModularPlayerController.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPlayerController: public APlayerController
{
	GENERATED_BODY()
public:

	AModularPlayerController(const FObjectInitializer& Initializer);

	//~Begin Actor interface
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End Actor interface

	//~Begin PlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~End PlayerController interface
	
};
