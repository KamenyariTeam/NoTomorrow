// © 2025 Kamenyari. All rights reserved.

#include "Combat/NotoDamageEffect.h"

#include "Combat/NotoHealthSet.h"
#include "Development/NotoGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoDamageEffect)

UNotoDamageEffect::UNotoDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataTag = NotoGameplayTags::Data_Damage;

	FGameplayModifierInfo& Modifier = Modifiers.AddDefaulted_GetRef();
	Modifier.Attribute = UNotoHealthSet::GetIncomingDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);
}
