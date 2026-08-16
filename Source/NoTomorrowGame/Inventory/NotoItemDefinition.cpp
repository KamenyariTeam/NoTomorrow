// © 2025 Kamenyari. All rights reserved.

#include "Inventory/NotoItemDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoItemDefinition)

int32 UNotoItemDefinition::GetMaxStackSize() const
{
	return ItemType == ENotoItemType::MainWeapon || UsesMagazine() ? 1 : FMath::Max(1, MaxStackSize);
}

bool UNotoItemDefinition::UsesMagazine() const
{
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	return bWeapon && GetMagazineCapacity() > 0;
}

#if WITH_EDITOR
EDataValidationResult UNotoItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);
	const bool bWeapon = ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon;
	const bool bHasMagazineData = MagazineCapacity != 0;

	auto AddError = [&Result, &Context](const FText& Error)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(Error);
	};

	if (!bWeapon && bHasMagazineData)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "MagazineOnNonWeapon",
		                   "Magazine data is only valid on main or secondary weapons."));
	}
	if (MagazineCapacity < 0)
	{
		AddError(NSLOCTEXT("NotoItemDefinition", "NegativeMagazineCapacity", "MagazineCapacity cannot be negative."));
	}
	if (ItemType == ENotoItemType::SecondaryWeapon && MaxStackSize > 1 && bHasMagazineData)
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
