// © 2025 Kamenyari. All rights reserved.

#include "Combat/NotoHealthSet.h"

#include "Development/NotoGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoHealthSet)

UNotoHealthSet::UNotoHealthSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
}

void UNotoHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UNotoHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNotoHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UNotoHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNotoHealthSet, Health, OldValue);

	const float PreviousHealth = OldValue.GetCurrentValue();
	const float NewHealth = GetHealth();
	if (NewHealth >= PreviousHealth)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystem = GetOwningAbilitySystemComponent();
	FNotoDamageEvent Event;
	Event.Target = AbilitySystem ? AbilitySystem->GetAvatarActor() : GetOwningActor();
	Event.Damage = PreviousHealth - NewHealth;
	Event.PreviousHealth = PreviousHealth;
	Event.NewHealth = NewHealth;
	DamageReceived.Broadcast(Event);
}

void UNotoHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNotoHealthSet, MaxHealth, OldValue);
}

void UNotoHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UNotoHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetIncomingDamageAttribute())
	{
		return;
	}

	const float RequestedDamage = FMath::Max(0.0f, GetIncomingDamage());
	SetIncomingDamage(0.0f);

	const float PreviousHealth = GetHealth();
	const float ActualDamage = FMath::Min(RequestedDamage, PreviousHealth);
	if (ActualDamage <= 0.0f)
	{
		return;
	}

	const float NewHealth = FMath::Clamp(PreviousHealth - ActualDamage, 0.0f, GetMaxHealth());
	SetHealth(NewHealth);

	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* AbilitySystem = GetOwningAbilitySystemComponent();
	FNotoDamageEvent Event;
	Event.Target = AbilitySystem ? AbilitySystem->GetAvatarActor() : GetOwningActor();
	Event.Instigator = Context.GetOriginalInstigator();
	Event.DamageCauser = Context.GetEffectCauser();
	Event.SourceObject = Context.GetSourceObject();
	Event.Damage = ActualDamage;
	Event.PreviousHealth = PreviousHealth;
	Event.NewHealth = NewHealth;
	if (const FHitResult* HitResult = Context.GetHitResult())
	{
		Event.HitResult = *HitResult;
	}

	if (AbilitySystem && NewHealth <= 0.0f)
	{
		const EGameplayTagReplicationState ReplicationState = AbilitySystem->GetOwnerRole() == ROLE_Authority
			                                                      ? EGameplayTagReplicationState::TagOnly
			                                                      : EGameplayTagReplicationState::None;
		AbilitySystem->SetLooseGameplayTagCount(NotoGameplayTags::State_Dead, 1, ReplicationState);
	}
	DamageReceived.Broadcast(Event);
}
