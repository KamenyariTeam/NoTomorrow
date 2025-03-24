#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "ModularPlayerState.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPlayerState: public APlayerState
{
	GENERATED_BODY()
public:

	//~Begin Actor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End Actor interface

	//~Begin PlayerState interface
	virtual void Reset() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	//~End PlayerState interface
};
