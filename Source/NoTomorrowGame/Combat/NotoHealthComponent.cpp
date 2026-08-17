// © 2025 Kamenyari. All rights reserved.

#include "Combat/NotoHealthComponent.h"

#include "AbilitySystemComponent.h"
#include "Development/NotoGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoHealthComponent)

UNotoHealthComponent::UNotoHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNotoHealthComponent::InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	if (AbilitySystem == InAbilitySystem && HealthSet.IsValid())
	{
		return;
	}

	UninitializeFromAbilitySystem();
	if (!ensure(InAbilitySystem))
	{
		return;
	}

	const UNotoHealthSet* FoundHealthSet = InAbilitySystem->GetSet<UNotoHealthSet>();
	if (!ensureMsgf(FoundHealthSet, TEXT("Ability system on %s has no UNotoHealthSet."), *GetNameSafe(GetOwner())))
	{
		return;
	}

	AbilitySystem = InAbilitySystem;
	HealthSet = const_cast<UNotoHealthSet*>(FoundHealthSet);
	if (!bHealthInitialized)
	{
		const float MaxHealth = FMath::Max(1.0f, InitialMaxHealth);
		InAbilitySystem->SetNumericAttributeBase(UNotoHealthSet::GetMaxHealthAttribute(), MaxHealth);
		InAbilitySystem->SetNumericAttributeBase(UNotoHealthSet::GetHealthAttribute(), MaxHealth);
		InAbilitySystem->RemoveLooseGameplayTag(NotoGameplayTags::State_Dead);
		bHealthInitialized = true;
	}

	DamageReceivedHandle = FoundHealthSet->OnDamageReceived().AddUObject(this, &ThisClass::HandleDamageReceived);
	bDead = InAbilitySystem->HasMatchingGameplayTag(NotoGameplayTags::State_Dead) || FoundHealthSet->GetHealth() <= 0.0f;
}

void UNotoHealthComponent::UninitializeFromAbilitySystem()
{
	if (HealthSet.IsValid() && DamageReceivedHandle.IsValid())
	{
		HealthSet->OnDamageReceived().Remove(DamageReceivedHandle);
	}
	DamageReceivedHandle.Reset();
	HealthSet.Reset();
	AbilitySystem.Reset();
}

float UNotoHealthComponent::GetHealth() const
{
	return HealthSet.IsValid() ? HealthSet->GetHealth() : 0.0f;
}

float UNotoHealthComponent::GetMaxHealth() const
{
	return HealthSet.IsValid() ? HealthSet->GetMaxHealth() : 0.0f;
}

bool UNotoHealthComponent::IsDead() const
{
	return bDead || (AbilitySystem.IsValid() && AbilitySystem->HasMatchingGameplayTag(NotoGameplayTags::State_Dead));
}

void UNotoHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeFromAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UNotoHealthComponent::HandleDamageReceived(const FNotoDamageEvent& DamageEvent)
{
	OnDamageReceived.Broadcast(DamageEvent);
	if (bDead || DamageEvent.NewHealth > 0.0f)
	{
		return;
	}

	bDead = true;
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->DisableMovement();
	}
	OnDeathStarted.Broadcast(DamageEvent);
}
