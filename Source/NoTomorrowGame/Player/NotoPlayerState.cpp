// © 2025 Kamenyari. All rights reserved.

#include "NotoPlayerState.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerState)

ANotoPlayerState::ANotoPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* ANotoPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
