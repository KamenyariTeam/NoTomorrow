// © 2025 Kamenyari. All rights reserved.

#include "Inventory/NotoItemDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoItemDefinition)

int32 UNotoItemDefinition::GetMaxStackSize() const
{
	return ItemType == ENotoItemType::MainWeapon || IsMagazine() || UsesMagazine() ? 1 : FMath::Max(1, MaxStackSize);
}

bool UNotoItemDefinition::UsesMagazine() const
{
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	return bWeapon && AmmoFeedType == ENotoAmmoFeedType::DetachableMagazine;
}

bool UNotoItemDefinition::UsesInternalAmmo() const
{
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	return bWeapon && AmmoFeedType == ENotoAmmoFeedType::Internal;
}

#if WITH_EDITOR
EDataValidationResult UNotoItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	const int32 FirearmCapacity = UsesMagazine() && StandardMagazineDefinition
		                              ? StandardMagazineDefinition->GetMagazineCapacity()
		                              : GetInternalCapacity();

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
	if (IsMagazine() && (MagazineCapacity <= 0 || !AmmoFamily.IsValid() || MaxStackSize != 1))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidMagazine",
		                   "A magazine requires a family, positive capacity, and MaxStackSize equal to one."));
	}
	if (!IsMagazine() && MagazineCapacity != 0)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "CapacityOnNonMagazine",
		                   "MagazineCapacity is only valid on physical magazine definitions."));
	}
	if (IsLooseAmmunition() && !AmmoFamily.IsValid())
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "LooseAmmoWithoutFamily",
		                   "Loose ammunition requires an ammo family."));
	}
	if (UsesMagazine()
		&& (!AmmoFamily.IsValid() || !StandardMagazineDefinition || !StandardMagazineDefinition->IsMagazine()
			|| StandardMagazineDefinition->GetAmmoFamily() != AmmoFamily))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidStandardMagazine",
		                   "A detachable-magazine weapon requires a standard magazine from the same ammo family."));
	}
	if (!UsesMagazine() && StandardMagazineDefinition)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "UnexpectedStandardMagazine",
		                   "Only detachable-magazine weapons may specify a standard magazine."));
	}
	if (UsesInternalAmmo() && (!AmmoFamily.IsValid() || InternalCapacity <= 0))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidInternalFeed",
		                   "An internal-feed weapon requires an ammo family and positive internal capacity."));
	}
	if (!UsesInternalAmmo() && InternalCapacity != 0)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "CapacityOnNonInternalFeed",
		                   "InternalCapacity is only valid on internal-feed weapons."));
	}
	if (IsFirearm() && (AmmoFeedType == ENotoAmmoFeedType::None || FirearmDamage <= 0.0f || FirearmRange <= 0.0f
		|| AmmoPerShot <= 0 || AmmoPerShot > FirearmCapacity || FireInterval < 0.0f || GunfireNoiseRange < 0.0f))
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "InvalidFirearm",
		                   "A firearm requires an ammo feed, positive damage/range/ammo per shot, valid fire interval/noise range, and enough loaded capacity for one shot."));
	}
	if (ItemType == ENotoItemType::SecondaryWeapon && MaxStackSize > 1 && AmmoFeedType != ENotoAmmoFeedType::None)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "StackedSecondaryMagazine",
		                   "A secondary weapon with magazine data cannot be stackable."));
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

	return Result;
}
#endif
