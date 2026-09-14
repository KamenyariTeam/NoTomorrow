// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Combat/NotoHealthSet.h"
#include "Components/ActorComponent.h"
#include "NotoHealthComponent.generated.h"

class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNotoDamageEventSignature, const FNotoDamageEvent&, DamageEvent);

/** Binds an avatar to GAS health attributes and exposes damage/death events to presentation. */
UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoHealthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem);
	void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintPure, Category = "Noto|Health")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Noto|Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Noto|Health")
	bool IsDead() const;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Noto|Health")
	FNotoDamageEventSignature OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Noto|Health")
	FNotoDamageEventSignature OnDeathStarted;

private:
	void HandleDamageReceived(const FNotoDamageEvent& DamageEvent);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noto|Health", Meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float InitialMaxHealth = 100.0f;

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
	TWeakObjectPtr<UNotoHealthSet> HealthSet;
	FDelegateHandle DamageReceivedHandle;
	bool bHealthInitialized = false;
	bool bDead = false;
};
