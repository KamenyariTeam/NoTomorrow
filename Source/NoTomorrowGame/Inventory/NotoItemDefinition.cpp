// © 2025 Kamenyari. All rights reserved.

#include "Inventory/NotoItemDefinition.h"

#include "Character/NotoEquippedItemActor.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoItemDefinition)

int32 UNotoItemDefinition::GetMaxStackSize() const
{
	return ItemType == ENotoItemType::MainWeapon || UsesAmmunition() ? 1 : FMath::Max(1, MaxStackSize);
}

FGameplayTag UNotoItemDefinition::GetAmmoFamily() const
{
	return IsAmmunition()
		       ? AmmoFamily
		       : (AmmunitionDefinition && AmmunitionDefinition->IsAmmunition()
		              ? AmmunitionDefinition->AmmoFamily
		              : FGameplayTag());
}

bool UNotoItemDefinition::UsesAmmunition() const
{
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	return bWeapon && AmmoFeedType != ENotoAmmoFeedType::None;
}

const FNotoEquippedItemAnimation* UNotoItemDefinition::FindEquippedAnimation(FGameplayTag ActionTag) const
{
	return EquippedAnimations.FindByPredicate([ActionTag](const FNotoEquippedItemAnimation& Animation)
	{
		return Animation.ActionTag == ActionTag;
	});
}

#if WITH_EDITOR
EDataValidationResult UNotoItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	const int32 FirearmCapacity = GetAmmoCapacity();

	auto AddError = [&Result, &Context](const FText& Error)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(Error);
	};

	if (!bWeapon && AmmoFeedType != ENotoAmmoFeedType::None)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "FeedOnNonWeapon",
		                   "AmmoFeedType is only valid on main or secondary weapons."));
	}
	if (IsAmmunition() && (!AmmoFamily.IsValid() || MaxStackSize <= 1))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidAmmunition",
		                   "Ammunition requires an ammo family and a stack size greater than one."));
	}
	if (UsesAmmunition()
		&& (!AmmunitionDefinition || !AmmunitionDefinition->IsAmmunition()
			|| AmmunitionDefinition->GetMaxStackSize() <= 1 || AmmoCapacity <= 0))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidWeaponAmmunition",
		                   "An ammunition-using weapon requires stackable ammunition and positive capacity."));
	}
	if (!UsesAmmunition() && (AmmunitionDefinition || AmmoCapacity != 0))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "UnexpectedWeaponAmmunition",
		                   "Only ammunition-using weapons may specify ammunition or capacity."));
	}
	if (IsFirearm() && (AmmoFeedType == ENotoAmmoFeedType::None || FirearmDamage <= 0.0f || FirearmRange <= 0.0f
		|| AmmoPerShot <= 0 || AmmoPerShot > FirearmCapacity || FireInterval < 0.0f || GunfireNoiseRange < 0.0f))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidFirearm",
		                   "A firearm requires an ammo feed, positive damage/range/ammo per shot, valid fire interval/noise range, and enough loaded capacity for one shot."));
	}
	if (ItemType == ENotoItemType::SecondaryWeapon && MaxStackSize > 1 && AmmoFeedType != ENotoAmmoFeedType::None)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "StackedSecondaryWeapon",
		                   "An ammunition-using secondary weapon cannot be stackable."));
	}
	if (ItemType == ENotoItemType::MainWeapon && MaxStackSize > 1)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "StackedMainWeapon",
		                   "Main weapons must be authored with MaxStackSize equal to one."));
	}
	if (MaxStackSize < 1)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidStackSize", "MaxStackSize must be at least one."));
	}
	if (EquippedActorClass && EquippedSocketName.IsNone())
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "PresentationWithoutSocket",
		                   "An equipped item actor requires an attachment socket name."));
	}
	TSet<FGameplayTag> AnimationTags;
	for (const FNotoEquippedItemAnimation& Animation : EquippedAnimations)
	{
		if (!EquippedActorClass || !Animation.ActionTag.IsValid() || AnimationTags.Contains(Animation.ActionTag))
		{
			AddError(NSLOCTEXT("NotoItemDefinition", "InvalidEquippedAnimation",
			                   "Equipped animations require an equipped actor class and a unique action tag."));
		}
		AnimationTags.Add(Animation.ActionTag);
	}

	return Result;
}
#endif
