// © 2025 Kamenyari. All rights reserved.

#include "Combat/NotoHealthSet.h"

#include "Development/NotoGameplayTags.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoHealthSet)

UNotoHealthSet::UNotoHealthSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
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
		AbilitySystem->AddLooseGameplayTag(NotoGameplayTags::State_Dead);
	}
	DamageReceived.Broadcast(Event);
}
