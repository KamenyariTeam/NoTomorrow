#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

#include "HUDComponent.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API UHUDComponent: public UGameFrameworkComponent
{
	GENERATED_BODY()
public:

	template <typename T = AHUD UE_REQUIRES(TPointerIsConvertibleFromTo<T, AHUD>::Value)>
	T* GetHUD() const
	{
		return Cast<T>(GetOwner(), ECastCheckedType::NullAllowed);
	}

	template <typename T = AHUD UE_REQUIRES(TPointerIsConvertibleFromTo<T, AHUD>::Value)>
	T* GetHUDChecked() const
	{
		return CastChecked<T>(GetOwner(), ECastCheckedType::NullAllowed);
	}

	template <typename T = APlayerController UE_REQUIRES(TPointerIsConvertibleFromTo<T, APlayerController>::Value)>
	T* GetOwningPlayer() const
	{
		return GetHUDChecked<AHUD>()->GetOwningPlayerController();
	}

	template <typename T = APawn UE_REQUIRES(TPointerIsConvertibleFromTo<T, APawn>::Value)>
	T* GetOwningPlayerPawn() const
	{
		return GetHUDChecked<AHUD>()->GetOwningPawn();
	}

public:

	virtual void DrawHUD(UCanvas* Canvas, UCanvas* DebugCanvas) {}
	virtual void ReceivePlayerPawn(APawn* NewPawn) {}
};
