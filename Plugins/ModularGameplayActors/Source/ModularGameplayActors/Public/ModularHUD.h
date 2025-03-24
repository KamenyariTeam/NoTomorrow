#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "ModularHUD.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularHUD: public AHUD
{
	GENERATED_BODY()
public:

	//~Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End AActor interface

	virtual void DrawHUD() override;
	virtual void ReceivePlayerPawn(APawn* NewPawn);
	
};
