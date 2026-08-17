// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "NotoCombatDummy.generated.h"

class UAbilitySystemComponent;
class UNotoHealthComponent;
class UNotoHealthSet;

/** Placeable GAS target for combat prototyping and automated tests. */
UCLASS(Blueprintable)
class NOTOMORROWGAME_API ANotoCombatDummy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANotoCombatDummy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Noto|Health")
	UNotoHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UNotoHealthSet* GetHealthSet() const { return HealthSet; }
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Abilities", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UNotoHealthSet> HealthSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noto|Health", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoHealthComponent> HealthComponent;
};
