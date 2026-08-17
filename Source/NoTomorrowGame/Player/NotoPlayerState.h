// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "NotoPlayerState.generated.h"

class UAbilitySystemComponent;
class UNotoInventoryComponent;

/** Persistent owner of the player's gameplay abilities and effects. */
UCLASS()
class NOTOMORROWGAME_API ANotoPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANotoPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	UNotoInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Abilities", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Inventory", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoInventoryComponent> InventoryComponent;
};
