// © 2025 Kamenyari. All rights reserved.

#include "Combat/NotoFirearmComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Combat/NotoDamageEffect.h"
#include "Development/NotoGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "Inventory/NotoInventoryComponent.h"
#include "Inventory/NotoItemDefinition.h"
#include "Perception/AISense_Hearing.h"
#include "Player/NotoPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoFirearmComponent)

namespace NotoFirearmComponent
{
#if !UE_BUILD_SHIPPING
	static TAutoConsoleVariable<int32> CVarDebugTraces(
		TEXT("Noto.Firearm.DebugTraces"),
		0,
		TEXT("Draw firearm hitscan traces. 0: disabled, 1: enabled."),
		ECVF_Cheat);
#endif
}

UNotoFirearmComponent::UNotoFirearmComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UNotoFirearmComponent::TryFire()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	UNotoInventoryComponent* Inventory = GetInventory();
	UAbilitySystemComponent* SourceAbilitySystem = GetAbilitySystem();
	UWorld* World = GetWorld();
	if (!Pawn || !Inventory || !SourceAbilitySystem || !World
		|| SourceAbilitySystem->HasMatchingGameplayTag(NotoGameplayTags::State_Dead))
	{
		return false;
	}

	FNotoItemInstance Weapon;
	if (!Inventory->GetActiveItem(Weapon) || !Weapon.Definition || !Weapon.Definition->IsFirearm())
	{
		return false;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime < NextAllowedFireTime
		|| !Inventory->ConsumeLoadedAmmo(Weapon.InstanceId, Weapon.Definition->GetAmmoPerShot()))
	{
		return false;
	}
	NextAllowedFireTime = CurrentTime + Weapon.Definition->GetFireInterval();

	const FVector TraceStart = Pawn->GetActorLocation()
		+ FVector::UpVector * Weapon.Definition->GetFirearmTraceHeight();
	const FVector TraceEnd = TraceStart + Pawn->GetActorForwardVector() * Weapon.Definition->GetFirearmRange();
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NotoFirearm), false, Pawn);
	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		Weapon.Definition->GetFirearmTraceChannel(),
		QueryParams);

#if !UE_BUILD_SHIPPING
	if (NotoFirearmComponent::CVarDebugTraces.GetValueOnGameThread() != 0)
	{
		const FVector DebugTraceEnd = bHit ? HitResult.ImpactPoint : TraceEnd;
		DrawDebugLine(World, TraceStart, DebugTraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
		if (bHit)
		{
			DrawDebugPoint(World, HitResult.ImpactPoint, 12.0f, FColor::Red, false, 1.0f);
		}
	}
#endif

	if (bHit)
	{
		ApplyDamage(HitResult, *Weapon.Definition, *SourceAbilitySystem);
	}

	if (Weapon.Definition->GetGunfireNoiseRange() > 0.0f)
	{
		UAISense_Hearing::ReportNoiseEvent(
			World,
			TraceStart,
			1.0f,
			Pawn,
			Weapon.Definition->GetGunfireNoiseRange(),
			NotoGameplayTags::NoiseTag_Gunfire.GetTag().GetTagName());
	}
	return true;
}

bool UNotoFirearmComponent::TryReload()
{
	UNotoInventoryComponent* Inventory = GetInventory();
	FNotoItemInstance Weapon;
	int32 ReloadedRounds = 0;
	return Inventory
		&& Inventory->GetActiveItem(Weapon)
		&& Weapon.Definition
		&& Weapon.Definition->IsFirearm()
		&& Inventory->ReloadItem(Weapon.InstanceId, ReloadedRounds);
}

UNotoInventoryComponent* UNotoFirearmComponent::GetInventory() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const ANotoPlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<ANotoPlayerState>() : nullptr;
	return PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
}

UAbilitySystemComponent* UNotoFirearmComponent::GetAbilitySystem() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

void UNotoFirearmComponent::ApplyDamage(
	const FHitResult& HitResult,
	const UNotoItemDefinition& Definition,
	UAbilitySystemComponent& SourceAbilitySystem) const
{
	UAbilitySystemComponent* TargetAbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		HitResult.GetActor());
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!TargetAbilitySystem || !Pawn)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystem.MakeEffectContext();
	EffectContext.AddInstigator(Pawn, Pawn);
	EffectContext.AddSourceObject(&Definition);
	EffectContext.AddHitResult(HitResult, true);

	FGameplayEffectSpecHandle DamageSpec = SourceAbilitySystem.MakeOutgoingSpec(
		UNotoDamageEffect::StaticClass(),
		1.0f,
		EffectContext);
	if (!DamageSpec.IsValid())
	{
		return;
	}

	DamageSpec.Data->SetSetByCallerMagnitude(NotoGameplayTags::Data_Damage, Definition.GetFirearmDamage());
	SourceAbilitySystem.ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetAbilitySystem);
}
