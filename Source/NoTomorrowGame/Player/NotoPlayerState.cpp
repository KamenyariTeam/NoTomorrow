// © 2025 Kamenyari. All rights reserved.

#include "NotoPlayerState.h"

#include "AbilitySystemComponent.h"
#include "Combat/NotoHealthSet.h"
#include "Inventory/NotoInventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerState)

ANotoPlayerState::ANotoPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	HealthSet = CreateDefaultSubobject<UNotoHealthSet>(TEXT("HealthSet"));

	InventoryComponent = CreateDefaultSubobject<UNotoInventoryComponent>(TEXT("Inventory"));
}

UAbilitySystemComponent* ANotoPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
