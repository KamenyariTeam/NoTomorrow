// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Engine/HitResult.h"
#include "NotoHealthSet.generated.h"

#define NOTO_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Context retained while a damage effect is being executed. */
USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoDamageEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> DamageCauser = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FHitResult HitResult;

	UPROPERTY(BlueprintReadOnly)
	float Damage = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PreviousHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float NewHealth = 0.0f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FNotoDamageReceivedNative, const FNotoDamageEvent&);

/** GAS attributes shared by player and NPC combatants. */
UCLASS()
class NOTOMORROWGAME_API UNotoHealthSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UNotoHealthSet();

	NOTO_ATTRIBUTE_ACCESSORS(UNotoHealthSet, Health)
	NOTO_ATTRIBUTE_ACCESSORS(UNotoHealthSet, MaxHealth)
	NOTO_ATTRIBUTE_ACCESSORS(UNotoHealthSet, IncomingDamage)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	FNotoDamageReceivedNative& OnDamageReceived() const { return DamageReceived; }

private:
	UPROPERTY(BlueprintReadOnly, Category = "Noto|Health", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, Category = "Noto|Health", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	/** Meta attribute consumed immediately by PostGameplayEffectExecute. */
	UPROPERTY()
	FGameplayAttributeData IncomingDamage;

	mutable FNotoDamageReceivedNative DamageReceived;
};

#undef NOTO_ATTRIBUTE_ACCESSORS
