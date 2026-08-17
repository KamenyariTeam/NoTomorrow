// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "NotoFirearmComponent.generated.h"

class UAbilitySystemComponent;
class UNotoInventoryComponent;
class UNotoItemDefinition;

/** Executes hitscan fire and reloads the active inventory weapon without per-frame work. */
UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoFirearmComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoFirearmComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Noto|Weapon")
	bool TryFire();

	UFUNCTION(BlueprintCallable, Category = "Noto|Weapon")
	bool TryReload();

private:
	UNotoInventoryComponent* GetInventory() const;
	UAbilitySystemComponent* GetAbilitySystem() const;
	void ApplyDamage(const FHitResult& HitResult, const UNotoItemDefinition& Definition,
	                 UAbilitySystemComponent& SourceAbilitySystem) const;

	double NextAllowedFireTime = 0.0;
};
