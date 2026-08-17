// © 2025 Kamenyari. All rights reserved.

#include "Combat/NotoCombatDummy.h"

#include "AbilitySystemComponent.h"
#include "Combat/NotoHealthComponent.h"
#include "Combat/NotoHealthSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoCombatDummy)

ANotoCombatDummy::ANotoCombatDummy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	HealthSet = CreateDefaultSubobject<UNotoHealthSet>(TEXT("HealthSet"));
	HealthComponent = CreateDefaultSubobject<UNotoHealthComponent>(TEXT("HealthComponent"));
}

UAbilitySystemComponent* ANotoCombatDummy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ANotoCombatDummy::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}
