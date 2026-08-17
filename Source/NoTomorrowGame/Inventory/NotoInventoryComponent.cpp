// © 2025 Kamenyari. All rights reserved.

#include "Inventory/NotoInventoryComponent.h"

#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/NotoItemDefinition.h"
#include "Inventory/NotoItemPickup.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoInventoryComponent)

UNotoInventoryComponent::FScopedMutation::FScopedMutation(UNotoInventoryComponent& InInventory)
	: Inventory(InInventory)
{
	++Inventory.MutationDepth;
}

UNotoInventoryComponent::FScopedMutation::~FScopedMutation()
{
	check(Inventory.MutationDepth > 0);
	if (--Inventory.MutationDepth == 0)
	{
		Inventory.FlushMutationNotifications();
	}
}

UNotoInventoryComponent::UNotoInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	DroppedPickupClass = ANotoItemPickup::StaticClass();
}

bool UNotoInventoryComponent::AddItem(UNotoItemDefinition* Definition, int32 Quantity, int32 LoadedAmmo,
                                      FGuid& OutItemInstanceId)
{
	FScopedMutation Mutation(*this);
	return AddItemInternal(Definition, Quantity, LoadedAmmo, FGuid(), OutItemInstanceId);
}

bool UNotoInventoryComponent::AddItemInternal(
	UNotoItemDefinition* Definition,
	int32 Quantity,
	int32 LoadedAmmo,
	FGuid PreferredItemInstanceId,
	FGuid& OutItemInstanceId)
{
	OutItemInstanceId.Invalidate();
	if (!Definition || Quantity <= 0 || !Definition->GetPrimaryAssetId().IsValid())
	{
		return false;
	}

	const FPrimaryAssetId DefinitionId = Definition->GetPrimaryAssetId();
	const int32 MaxStackSize = Definition->GetMaxStackSize();
	int32 RemainingQuantity = Quantity;

	auto FillStack = [Definition, DefinitionId, MaxStackSize, &RemainingQuantity, &OutItemInstanceId
		](FNotoItemInstance& Item)
	{
		if (RemainingQuantity <= 0 || Item.DefinitionId != DefinitionId || Item.Quantity >= MaxStackSize)
		{
			return;
		}

		Item.Definition = Definition;
		if (!OutItemInstanceId.IsValid())
		{
			OutItemInstanceId = Item.InstanceId;
		}

		const int32 AddedQuantity = FMath::Min(RemainingQuantity, MaxStackSize - Item.Quantity);
		Item.Quantity += AddedQuantity;
		RemainingQuantity -= AddedQuantity;
	};

	if (PreferredItemInstanceId.IsValid())
	{
		if (FNotoItemInstance* PreferredItem = FindItem(PreferredItemInstanceId))
		{
			FillStack(*PreferredItem);
		}
	}

	for (FNotoItemInstance& Item : Items)
	{
		if (Item.InstanceId != PreferredItemInstanceId)
		{
			FillStack(Item);
		}
	}

	while (RemainingQuantity > 0)
	{
		FNotoItemInstance& Item = Items.AddDefaulted_GetRef();
		Item.InstanceId = FGuid::NewGuid();
		Item.DefinitionId = DefinitionId;
		Item.Definition = Definition;
		Item.Quantity = FMath::Min(RemainingQuantity, MaxStackSize);
		InitializeItemAmmunition(Item, LoadedAmmo);
		RemainingQuantity -= Item.Quantity;

		if (!OutItemInstanceId.IsValid())
		{
			OutItemInstanceId = Item.InstanceId;
		}
	}

	MarkInventoryDirty();
	return true;
}

bool UNotoInventoryComponent::CollectItem(
	UNotoItemDefinition* Definition,
	int32 Quantity,
	int32 LoadedAmmo,
	FGuid& OutItemInstanceId,
	int32& OutRemainingLoadedAmmo,
	bool bMakeCollectedItemActive)
{
	OutItemInstanceId.Invalidate();
	OutRemainingLoadedAmmo = 0;
	FCollectionPlan Plan;
	if (!Definition || !BuildCollectionPlan(*Definition, Quantity, Plan))
	{
		return false;
	}

	FScopedMutation Mutation(*this);
	const ENotoEquipmentSlot PreviousActiveSlot = ActiveSlot;
	const bool bAdded = AddItemInternal(Definition, Quantity, LoadedAmmo, Plan.PreferredItemInstanceId,
	                                    OutItemInstanceId);
	check(bAdded && OutItemInstanceId.IsValid());

	switch (Plan.Action)
	{
	case ECollectionAction::FillEquippedStack:
		check(OutItemInstanceId == Plan.PreferredItemInstanceId);
		return !bMakeCollectedItemActive || SetActiveSlot(Plan.Slot);
	case ECollectionAction::Equip:
		{
			if (!EquipItem(OutItemInstanceId, Plan.Slot, false))
			{
				return false;
			}
			return bMakeCollectedItemActive || SetActiveSlot(PreviousActiveSlot);
		}
	case ECollectionAction::AddOnly:
	default:
		return true;
	}
}

bool UNotoInventoryComponent::CollectItemInstance(
	const FNotoItemInstance& ItemInstance,
	FGuid& OutItemInstanceId,
	bool bMakeCollectedItemActive)
{
	OutItemInstanceId.Invalidate();
	FCollectionPlan Plan;
	if (!IsItemStateValid(ItemInstance)
		|| !BuildCollectionPlan(*ItemInstance.Definition, ItemInstance.Quantity, Plan))
	{
		return false;
	}

	FScopedMutation Mutation(*this);
	const ENotoEquipmentSlot PreviousActiveSlot = ActiveSlot;
	if (!AddItemInstanceInternal(ItemInstance, OutItemInstanceId))
	{
		return false;
	}

	if (Plan.Action != ECollectionAction::Equip)
	{
		return true;
	}
	if (!EquipItem(OutItemInstanceId, Plan.Slot, false))
	{
		return false;
	}
	return bMakeCollectedItemActive || SetActiveSlot(PreviousActiveSlot);
}

bool UNotoInventoryComponent::CanCollectItem(const UNotoItemDefinition* Definition, int32 Quantity,
                                             int32 LoadedAmmo) const
{
	FCollectionPlan Plan;
	return Definition && BuildCollectionPlan(*Definition, Quantity, Plan);
}

bool UNotoInventoryComponent::CanCollectItemInstance(const FNotoItemInstance& ItemInstance) const
{
	FCollectionPlan Plan;
	return IsItemStateValid(ItemInstance)
		&& !IsInstanceIdInUse(ItemInstance.InstanceId)
		&& (!ItemInstance.InsertedMagazine.IsValid()
			|| !IsInstanceIdInUse(ItemInstance.InsertedMagazine.InstanceId))
		&& BuildCollectionPlan(*ItemInstance.Definition, ItemInstance.Quantity, Plan);
}

bool UNotoInventoryComponent::BuildCollectionPlan(
	const UNotoItemDefinition& Definition,
	int32 Quantity,
	FCollectionPlan& OutPlan) const
{
	OutPlan = FCollectionPlan();
	if (Quantity <= 0 || !Definition.GetPrimaryAssetId().IsValid())
	{
		return false;
	}
	if (Definition.GetMaxStackSize() == 1 && Quantity != 1)
	{
		return false;
	}

	if (!IsEquipmentItem(Definition))
	{
		return true;
	}

	TArray<ENotoEquipmentSlot, TInlineAllocator<5>> CompatibleSlots;
	switch (Definition.GetItemType())
	{
	case ENotoItemType::MainWeapon:
		CompatibleSlots.Add(ENotoEquipmentSlot::MainWeapon);
		break;
	case ENotoItemType::SecondaryWeapon:
		CompatibleSlots.Add(ENotoEquipmentSlot::SecondaryWeapon);
		break;
	case ENotoItemType::Tool:
		for (int32 SlotValue = static_cast<int32>(ENotoEquipmentSlot::Tool5);
		     SlotValue >= static_cast<int32>(ENotoEquipmentSlot::Tool1);
		     --SlotValue)
		{
			CompatibleSlots.Add(static_cast<ENotoEquipmentSlot>(SlotValue));
		}
		break;
	default:
		return true;
	}

	const FPrimaryAssetId DefinitionId = Definition.GetPrimaryAssetId();
	const int32 MaxStackSize = Definition.GetMaxStackSize();

	for (const ENotoEquipmentSlot Slot : CompatibleSlots)
	{
		const FNotoEquippedItem* EquippedItem = FindEquipment(Slot);
		const FNotoItemInstance* Item = EquippedItem ? FindItem(EquippedItem->ItemInstanceId) : nullptr;
		if (Item && Item->DefinitionId == DefinitionId && Item->Quantity < MaxStackSize)
		{
			OutPlan.Action = ECollectionAction::FillEquippedStack;
			OutPlan.Slot = Slot;
			OutPlan.PreferredItemInstanceId = Item->InstanceId;
			return true;
		}
	}

	for (const ENotoEquipmentSlot Slot : CompatibleSlots)
	{
		if (!FindEquipment(Slot))
		{
			OutPlan.Action = ECollectionAction::Equip;
			OutPlan.Slot = Slot;
			return true;
		}
	}
	return true;
}

void UNotoInventoryComponent::InitializeItemAmmunition(FNotoItemInstance& Item, int32 LoadedAmmo)
{
	check(Item.Definition);
	const UNotoItemDefinition& Definition = *Item.Definition;
	if (Definition.IsMagazine())
	{
		Item.LoadedAmmo = ResolveLoadedAmmo(Definition, LoadedAmmo);
	}
	else if (Definition.UsesMagazine())
	{
		UNotoItemDefinition* MagazineDefinition = Definition.GetStandardMagazineDefinition();
		if (MagazineDefinition)
		{
			Item.InsertedMagazine.InstanceId = FGuid::NewGuid();
			Item.InsertedMagazine.DefinitionId = MagazineDefinition->GetPrimaryAssetId();
			Item.InsertedMagazine.Definition = MagazineDefinition;
			Item.InsertedMagazine.LoadedAmmo = ResolveLoadedAmmo(*MagazineDefinition, LoadedAmmo);
		}
	}
	else if (Definition.UsesInternalAmmo())
	{
		Item.LoadedAmmo = FMath::Clamp(
			LoadedAmmo < 0 ? Definition.GetInternalCapacity() : LoadedAmmo,
			0,
			Definition.GetInternalCapacity());
	}
}

bool UNotoInventoryComponent::IsItemStateValid(const FNotoItemInstance& Item)
{
	if (!Item.InstanceId.IsValid() || !Item.Definition || Item.DefinitionId != Item.Definition->GetPrimaryAssetId()
		|| Item.Quantity <= 0 || (Item.Definition->GetMaxStackSize() == 1 && Item.Quantity != 1))
	{
		return false;
	}

	if (Item.Definition->IsMagazine())
	{
		return Item.LoadedAmmo >= 0 && Item.LoadedAmmo <= Item.Definition->GetMagazineCapacity();
	}
	if (Item.Definition->UsesMagazine())
	{
		return Item.InsertedMagazine.IsEmpty()
			|| (Item.InsertedMagazine.Definition
				&& Item.InsertedMagazine.InstanceId != Item.InstanceId
				&& Item.InsertedMagazine.DefinitionId == Item.InsertedMagazine.Definition->GetPrimaryAssetId()
				&& Item.InsertedMagazine.Definition->IsMagazine()
				&& Item.InsertedMagazine.Definition->GetAmmoFamily() == Item.Definition->GetAmmoFamily()
				&& Item.InsertedMagazine.LoadedAmmo >= 0
				&& Item.InsertedMagazine.LoadedAmmo <= Item.InsertedMagazine.Definition->GetMagazineCapacity());
	}
	if (Item.Definition->UsesInternalAmmo())
	{
		return Item.LoadedAmmo >= 0 && Item.LoadedAmmo <= Item.Definition->GetInternalCapacity();
	}
	return Item.LoadedAmmo == 0 && !Item.InsertedMagazine.IsValid();
}

int32 UNotoInventoryComponent::ResolveLoadedAmmo(const UNotoItemDefinition& Definition, int32 LoadedAmmo)
{
	return Definition.IsMagazine()
		       ? FMath::Clamp(LoadedAmmo < 0 ? Definition.GetMagazineCapacity() : LoadedAmmo, 0,
		                      Definition.GetMagazineCapacity())
		       : 0;
}

bool UNotoInventoryComponent::EquipItem(FGuid ItemInstanceId, ENotoEquipmentSlot Slot, bool bDropReplacedItem)
{
	FScopedMutation Mutation(*this);
	FNotoItemInstance* Item = FindItem(ItemInstanceId);
	if (!Item || !Item->Definition || !IsCompatibleSlot(*Item->Definition, Slot))
	{
		return false;
	}

	if (const FNotoEquippedItem* ExistingEquipment = FindEquipment(Slot))
	{
		if (ExistingEquipment->ItemInstanceId == ItemInstanceId)
		{
			SetActiveSlot(Slot);
			return true;
		}

		if (bDropReplacedItem)
		{
			const FNotoItemInstance* ReplacedItem = FindItem(ExistingEquipment->ItemInstanceId);
			if (!ReplacedItem || !DropItem(ReplacedItem->InstanceId, ReplacedItem->Quantity))
			{
				return false;
			}
		}
		else
		{
			UnequipItem(Slot);
		}
	}

	Equipment.RemoveAll([ItemInstanceId](const FNotoEquippedItem& EquippedItem)
	{
		return EquippedItem.ItemInstanceId == ItemInstanceId;
	});

	FNotoEquippedItem& EquippedItem = Equipment.AddDefaulted_GetRef();
	EquippedItem.Slot = Slot;
	EquippedItem.ItemInstanceId = ItemInstanceId;
	ActiveSlot = Slot;
	MarkEquipmentDirty();
	return true;
}

bool UNotoInventoryComponent::UnequipItem(ENotoEquipmentSlot Slot)
{
	FScopedMutation Mutation(*this);
	const int32 RemovedCount = Equipment.RemoveAll([Slot](const FNotoEquippedItem& EquippedItem)
	{
		return EquippedItem.Slot == Slot;
	});

	if (RemovedCount == 0)
	{
		return false;
	}

	if (ActiveSlot == Slot)
	{
		ActiveSlot = ENotoEquipmentSlot::None;
	}
	MarkEquipmentDirty();
	return true;
}

bool UNotoInventoryComponent::SetActiveSlot(ENotoEquipmentSlot Slot)
{
	FScopedMutation Mutation(*this);
	if (Slot != ENotoEquipmentSlot::None && !FindEquipment(Slot))
	{
		return false;
	}

	if (ActiveSlot != Slot)
	{
		ActiveSlot = Slot;
		MarkEquipmentDirty();
	}
	return true;
}

bool UNotoInventoryComponent::DropItem(FGuid ItemInstanceId, int32 Quantity)
{
	FTransform DropTransform;
	return TryGetDefaultDropTransform(DropTransform) && DropItemAt(ItemInstanceId, Quantity, DropTransform);
}

bool UNotoInventoryComponent::DropItemAt(FGuid ItemInstanceId, int32 Quantity, const FTransform& DropTransform)
{
	FScopedMutation Mutation(*this);
	const FNotoItemInstance* Item = FindItem(ItemInstanceId);
	UWorld* World = GetWorld();
	if (!Item
		|| !Item->Definition
		|| !Item->Definition->IsDroppable()
		|| Quantity <= 0
		|| Quantity > Item->Quantity
		|| !DropTransform.IsValid()
		|| !World
		|| !DroppedPickupClass
		|| DroppedPickupClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return false;
	}

	ANotoItemPickup* Pickup = World->SpawnActorDeferred<ANotoItemPickup>(
		DroppedPickupClass,
		DropTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Pickup)
	{
		return false;
	}

	FNotoItemInstance DroppedItem = *Item;
	DroppedItem.Quantity = Quantity;
	if (Quantity < Item->Quantity)
	{
		DroppedItem.InstanceId = FGuid::NewGuid();
	}
	Pickup->InitializePickup(DroppedItem);
	Pickup->FinishSpawning(DropTransform);
	RemoveItemQuantity(ItemInstanceId, Quantity);
	return true;
}

bool UNotoInventoryComponent::CanDropItem(FGuid ItemInstanceId, int32 Quantity) const
{
	const FNotoItemInstance* Item = FindItem(ItemInstanceId);
	FTransform DropTransform;
	return Item
		&& Item->Definition
		&& Item->Definition->IsDroppable()
		&& Quantity > 0
		&& Quantity <= Item->Quantity
		&& GetWorld()
		&& DroppedPickupClass
		&& !DroppedPickupClass->HasAnyClassFlags(CLASS_Abstract)
		&& TryGetDefaultDropTransform(DropTransform);
}

bool UNotoInventoryComponent::SetLoadedAmmo(FGuid ItemInstanceId, int32 LoadedAmmo)
{
	FScopedMutation Mutation(*this);
	FNotoItemInstance* Item = FindItem(ItemInstanceId);
	if (!Item || !Item->Definition || LoadedAmmo < 0)
	{
		return false;
	}

	int32* MutableAmmo = nullptr;
	int32 Capacity = 0;
	if (Item->Definition->IsMagazine())
	{
		MutableAmmo = &Item->LoadedAmmo;
		Capacity = Item->Definition->GetMagazineCapacity();
	}
	else if (Item->Definition->UsesMagazine() && Item->InsertedMagazine.IsValid()
		&& Item->InsertedMagazine.Definition)
	{
		MutableAmmo = &Item->InsertedMagazine.LoadedAmmo;
		Capacity = Item->InsertedMagazine.Definition->GetMagazineCapacity();
	}
	else if (Item->Definition->UsesInternalAmmo())
	{
		MutableAmmo = &Item->LoadedAmmo;
		Capacity = Item->Definition->GetInternalCapacity();
	}

	if (!MutableAmmo || LoadedAmmo > Capacity)
	{
		return false;
	}
	if (*MutableAmmo != LoadedAmmo)
	{
		*MutableAmmo = LoadedAmmo;
		MarkInventoryDirty();
	}
	return true;
}

bool UNotoInventoryComponent::AddItemInstanceInternal(
	const FNotoItemInstance& ItemInstance,
	FGuid& OutItemInstanceId)
{
	OutItemInstanceId.Invalidate();
	if (!IsItemStateValid(ItemInstance)
		|| IsInstanceIdInUse(ItemInstance.InstanceId)
		|| (ItemInstance.InsertedMagazine.IsValid()
			&& IsInstanceIdInUse(ItemInstance.InsertedMagazine.InstanceId)))
	{
		return false;
	}

	if (ItemInstance.Definition->GetMaxStackSize() > 1)
	{
		return AddItemInternal(
			ItemInstance.Definition,
			ItemInstance.Quantity,
			ItemInstance.LoadedAmmo,
			FGuid(),
			OutItemInstanceId);
	}

	Items.Add(ItemInstance);
	OutItemInstanceId = ItemInstance.InstanceId;
	MarkInventoryDirty();
	return true;
}

bool UNotoInventoryComponent::ConsumeLoadedAmmo(FGuid ItemInstanceId, int32 Amount)
{
	FScopedMutation Mutation(*this);
	FNotoItemInstance* Item = FindItem(ItemInstanceId);
	if (!Item || !Item->Definition || !Item->Definition->IsFirearm() || Amount <= 0)
	{
		return false;
	}

	int32* MutableAmmo = Item->Definition->UsesMagazine()
		                     ? (Item->InsertedMagazine.IsValid() ? &Item->InsertedMagazine.LoadedAmmo : nullptr)
		                     : &Item->LoadedAmmo;
	if (!MutableAmmo || *MutableAmmo < Amount)
	{
		return false;
	}

	*MutableAmmo -= Amount;
	MarkInventoryDirty();
	return true;
}

bool UNotoInventoryComponent::ReloadItem(FGuid ItemInstanceId, int32& OutReloadedRounds)
{
	OutReloadedRounds = 0;
	FScopedMutation Mutation(*this);
	const int32 WeaponIndex = Items.IndexOfByPredicate([ItemInstanceId](const FNotoItemInstance& Item)
	{
		return Item.InstanceId == ItemInstanceId;
	});
	if (WeaponIndex == INDEX_NONE || !Items[WeaponIndex].Definition || !Items[WeaponIndex].Definition->IsFirearm())
	{
		return false;
	}

	const UNotoItemDefinition& WeaponDefinition = *Items[WeaponIndex].Definition;
	if (WeaponDefinition.UsesMagazine())
	{
		const int32 CurrentRounds = Items[WeaponIndex].InsertedMagazine.IsValid()
			                            ? Items[WeaponIndex].InsertedMagazine.LoadedAmmo
			                            : 0;
		int32 BestMagazineIndex = INDEX_NONE;
		int32 BestRounds = CurrentRounds;
		for (int32 Index = 0; Index < Items.Num(); ++Index)
		{
			const FNotoItemInstance& Candidate = Items[Index];
			if (Candidate.Definition
				&& Candidate.Definition->IsMagazine()
				&& Candidate.Definition->GetAmmoFamily() == WeaponDefinition.GetAmmoFamily()
				&& Candidate.LoadedAmmo > BestRounds)
			{
				BestMagazineIndex = Index;
				BestRounds = Candidate.LoadedAmmo;
			}
		}
		if (BestMagazineIndex == INDEX_NONE)
		{
			return false;
		}

		const FNotoItemInstance IncomingMagazine = Items[BestMagazineIndex];
		FNotoItemInstance OutgoingMagazine;
		const FNotoInsertedMagazine PreviousMagazine = Items[WeaponIndex].InsertedMagazine;
		if (PreviousMagazine.IsValid() && PreviousMagazine.LoadedAmmo > 0)
		{
			OutgoingMagazine.InstanceId = PreviousMagazine.InstanceId;
			OutgoingMagazine.DefinitionId = PreviousMagazine.DefinitionId;
			OutgoingMagazine.Definition = PreviousMagazine.Definition;
			OutgoingMagazine.Quantity = 1;
			OutgoingMagazine.LoadedAmmo = PreviousMagazine.LoadedAmmo;
		}

		Items.RemoveAtSwap(BestMagazineIndex, EAllowShrinking::No);
		FNotoItemInstance* Weapon = FindItem(ItemInstanceId);
		check(Weapon);
		Weapon->InsertedMagazine.InstanceId = IncomingMagazine.InstanceId;
		Weapon->InsertedMagazine.DefinitionId = IncomingMagazine.DefinitionId;
		Weapon->InsertedMagazine.Definition = IncomingMagazine.Definition;
		Weapon->InsertedMagazine.LoadedAmmo = IncomingMagazine.LoadedAmmo;
		if (OutgoingMagazine.InstanceId.IsValid())
		{
			Items.Add(MoveTemp(OutgoingMagazine));
		}

		OutReloadedRounds = IncomingMagazine.LoadedAmmo;
		MarkInventoryDirty();
		return true;
	}

	if (!WeaponDefinition.UsesInternalAmmo() || Items[WeaponIndex].LoadedAmmo >= WeaponDefinition.GetInternalCapacity())
	{
		return false;
	}
	const int32 LooseAmmoIndex = Items.IndexOfByPredicate([&WeaponDefinition](const FNotoItemInstance& Candidate)
	{
		return Candidate.Definition
			&& Candidate.Definition->IsLooseAmmunition()
			&& Candidate.Definition->GetAmmoFamily() == WeaponDefinition.GetAmmoFamily()
			&& Candidate.Quantity > 0;
	});
	if (LooseAmmoIndex == INDEX_NONE)
	{
		return false;
	}

	Items[WeaponIndex].LoadedAmmo += 1;
	Items[LooseAmmoIndex].Quantity -= 1;
	if (Items[LooseAmmoIndex].Quantity == 0)
	{
		Items.RemoveAtSwap(LooseAmmoIndex, EAllowShrinking::No);
	}
	OutReloadedRounds = 1;
	MarkInventoryDirty();
	return true;
}

bool UNotoInventoryComponent::GetItem(FGuid ItemInstanceId, FNotoItemInstance& OutItem) const
{
	if (const FNotoItemInstance* Item = FindItem(ItemInstanceId))
	{
		OutItem = *Item;
		return true;
	}
	return false;
}

bool UNotoInventoryComponent::GetEquippedItem(ENotoEquipmentSlot Slot, FNotoItemInstance& OutItem) const
{
	if (const FNotoEquippedItem* EquippedItem = FindEquipment(Slot))
	{
		return GetItem(EquippedItem->ItemInstanceId, OutItem);
	}
	return false;
}

bool UNotoInventoryComponent::GetActiveItem(FNotoItemInstance& OutItem) const
{
	return GetEquippedItem(ActiveSlot, OutItem);
}

int32 UNotoInventoryComponent::GetTotalQuantity(const UNotoItemDefinition* Definition) const
{
	if (!Definition)
	{
		return 0;
	}

	const FPrimaryAssetId DefinitionId = Definition->GetPrimaryAssetId();
	int32 TotalQuantity = 0;
	for (const FNotoItemInstance& Item : Items)
	{
		if (Item.DefinitionId == DefinitionId)
		{
			TotalQuantity += Item.Quantity;
		}
	}
	return TotalQuantity;
}

bool UNotoInventoryComponent::HasItemWithTag(FGameplayTag ItemTag, int32 MinimumQuantity) const
{
	if (!ItemTag.IsValid() || MinimumQuantity <= 0)
	{
		return false;
	}

	int32 TotalQuantity = 0;
	for (const FNotoItemInstance& Item : Items)
	{
		if (Item.Definition && Item.Definition->GetItemTags().HasTag(ItemTag))
		{
			TotalQuantity += Item.Quantity;
			if (TotalQuantity >= MinimumQuantity)
			{
				return true;
			}
		}
	}
	return false;
}

bool UNotoInventoryComponent::ResolveItemDefinitions()
{
	FScopedMutation Mutation(*this);
	UAssetManager& AssetManager = UAssetManager::Get();
	bool bAllResolved = true;
	bool bAnyDefinitionChanged = false;
	TArray<FNotoItemInstance> MigratedMagazines;

	auto ResolveDefinition = [&AssetManager](FPrimaryAssetId DefinitionId)
	{
		UNotoItemDefinition* Definition = Cast<UNotoItemDefinition>(AssetManager.GetPrimaryAssetObject(DefinitionId));
		if (!Definition)
		{
			Definition = Cast<UNotoItemDefinition>(AssetManager.GetPrimaryAssetPath(DefinitionId).TryLoad());
		}
		return Definition && Definition->GetPrimaryAssetId() == DefinitionId ? Definition : nullptr;
	};

	for (FNotoItemInstance& Item : Items)
	{
		if (!Item.Definition || Item.Definition->GetPrimaryAssetId() != Item.DefinitionId)
		{
			Item.Definition = ResolveDefinition(Item.DefinitionId);
			bAnyDefinitionChanged = true;
			if (!Item.Definition)
			{
				bAllResolved = false;
				continue;
			}
		}

		if (Item.Definition->UsesMagazine())
		{
			UNotoItemDefinition* StandardMagazine = Item.Definition->GetStandardMagazineDefinition();
			int32 LegacySpareRounds = FMath::Max(0, Item.ReserveAmmo);
			if (!Item.InsertedMagazine.IsValid())
			{
				if (!StandardMagazine)
				{
					bAllResolved = false;
					continue;
				}
				const int32 LegacyLoadedAmmo = FMath::Max(0, Item.LoadedAmmo);
				Item.InsertedMagazine.InstanceId = FGuid::NewGuid();
				Item.InsertedMagazine.DefinitionId = StandardMagazine->GetPrimaryAssetId();
				Item.InsertedMagazine.Definition = StandardMagazine;
				Item.InsertedMagazine.LoadedAmmo = FMath::Clamp(
					LegacyLoadedAmmo,
					0,
					StandardMagazine->GetMagazineCapacity());
				LegacySpareRounds += FMath::Max(0, LegacyLoadedAmmo - Item.InsertedMagazine.LoadedAmmo);
				Item.LoadedAmmo = 0;
				bAnyDefinitionChanged = true;
			}
			else if (!Item.InsertedMagazine.Definition
				|| Item.InsertedMagazine.Definition->GetPrimaryAssetId() != Item.InsertedMagazine.DefinitionId)
			{
				Item.InsertedMagazine.Definition = ResolveDefinition(Item.InsertedMagazine.DefinitionId);
				bAnyDefinitionChanged = true;
				if (!Item.InsertedMagazine.Definition)
				{
					bAllResolved = false;
					continue;
				}
			}
			else if (Item.LoadedAmmo != 0)
			{
				LegacySpareRounds += FMath::Max(0, Item.LoadedAmmo);
				Item.LoadedAmmo = 0;
				bAnyDefinitionChanged = true;
			}

			const int32 ClampedInsertedAmmo = FMath::Clamp(
				Item.InsertedMagazine.LoadedAmmo,
				0,
				Item.InsertedMagazine.Definition->GetMagazineCapacity());
			if (ClampedInsertedAmmo != Item.InsertedMagazine.LoadedAmmo)
			{
				Item.InsertedMagazine.LoadedAmmo = ClampedInsertedAmmo;
				bAnyDefinitionChanged = true;
			}

			if (LegacySpareRounds > 0 && StandardMagazine)
			{
				const int32 Capacity = StandardMagazine->GetMagazineCapacity();
				int32 RemainingRounds = LegacySpareRounds;
				while (Capacity > 0 && RemainingRounds > 0)
				{
					FNotoItemInstance& Magazine = MigratedMagazines.AddDefaulted_GetRef();
					Magazine.InstanceId = FGuid::NewGuid();
					Magazine.DefinitionId = StandardMagazine->GetPrimaryAssetId();
					Magazine.Definition = StandardMagazine;
					Magazine.Quantity = 1;
					Magazine.LoadedAmmo = FMath::Min(Capacity, RemainingRounds);
					RemainingRounds -= Magazine.LoadedAmmo;
				}
			}
			if (Item.ReserveAmmo != 0)
			{
				Item.ReserveAmmo = 0;
				bAnyDefinitionChanged = true;
			}
		}
		else if (Item.Definition->IsMagazine())
		{
			const int32 ClampedAmmo = FMath::Clamp(Item.LoadedAmmo, 0, Item.Definition->GetMagazineCapacity());
			if (ClampedAmmo != Item.LoadedAmmo)
			{
				Item.LoadedAmmo = ClampedAmmo;
				bAnyDefinitionChanged = true;
			}
		}
		else if (Item.Definition->UsesInternalAmmo())
		{
			const int32 ClampedAmmo = FMath::Clamp(Item.LoadedAmmo, 0, Item.Definition->GetInternalCapacity());
			if (ClampedAmmo != Item.LoadedAmmo)
			{
				Item.LoadedAmmo = ClampedAmmo;
				bAnyDefinitionChanged = true;
			}
		}
	}
	Items.Append(MoveTemp(MigratedMagazines));

	if (bAnyDefinitionChanged)
	{
		MarkInventoryDirty();
	}
	return bAllResolved;
}

bool UNotoInventoryComponent::IsEquipmentItem(const UNotoItemDefinition& Definition) const
{
	return Definition.GetItemType() == ENotoItemType::MainWeapon
		|| Definition.GetItemType() == ENotoItemType::SecondaryWeapon
		|| Definition.GetItemType() == ENotoItemType::Tool;
}

bool UNotoInventoryComponent::IsInstanceIdInUse(FGuid InstanceId) const
{
	return InstanceId.IsValid() && Items.ContainsByPredicate([InstanceId](const FNotoItemInstance& Item)
	{
		return Item.InstanceId == InstanceId || Item.InsertedMagazine.InstanceId == InstanceId;
	});
}

FNotoItemInstance* UNotoInventoryComponent::FindItem(FGuid ItemInstanceId)
{
	return Items.FindByPredicate([ItemInstanceId](const FNotoItemInstance& Item)
	{
		return Item.InstanceId == ItemInstanceId;
	});
}

const FNotoItemInstance* UNotoInventoryComponent::FindItem(FGuid ItemInstanceId) const
{
	return Items.FindByPredicate([ItemInstanceId](const FNotoItemInstance& Item)
	{
		return Item.InstanceId == ItemInstanceId;
	});
}

FNotoEquippedItem* UNotoInventoryComponent::FindEquipment(ENotoEquipmentSlot Slot)
{
	return Equipment.FindByPredicate([Slot](const FNotoEquippedItem& EquippedItem)
	{
		return EquippedItem.Slot == Slot;
	});
}

const FNotoEquippedItem* UNotoInventoryComponent::FindEquipment(ENotoEquipmentSlot Slot) const
{
	return Equipment.FindByPredicate([Slot](const FNotoEquippedItem& EquippedItem)
	{
		return EquippedItem.Slot == Slot;
	});
}

bool UNotoInventoryComponent::IsCompatibleSlot(const UNotoItemDefinition& Definition, ENotoEquipmentSlot Slot) const
{
	switch (Definition.GetItemType())
	{
	case ENotoItemType::MainWeapon:
		return Slot == ENotoEquipmentSlot::MainWeapon;
	case ENotoItemType::SecondaryWeapon:
		return Slot == ENotoEquipmentSlot::SecondaryWeapon;
	case ENotoItemType::Tool:
		return Slot >= ENotoEquipmentSlot::Tool1 && Slot <= ENotoEquipmentSlot::Tool5;
	default:
		return false;
	}
}

bool UNotoInventoryComponent::TryGetDefaultDropTransform(FTransform& OutDropTransform) const
{
	const APlayerState* PlayerState = Cast<APlayerState>(GetOwner());
	const APawn* Pawn = PlayerState ? PlayerState->GetPawn() : nullptr;
	if (!Pawn)
	{
		return false;
	}

	OutDropTransform = FTransform(Pawn->GetActorRotation(), Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 100.0f);
	return true;
}

void UNotoInventoryComponent::RemoveItemQuantity(FGuid ItemInstanceId, int32 Quantity)
{
	FNotoItemInstance* Item = FindItem(ItemInstanceId);
	check(Item && Quantity > 0 && Quantity <= Item->Quantity);

	Item->Quantity -= Quantity;
	if (Item->Quantity == 0)
	{
		const int32 RemovedEquipment = Equipment.RemoveAll([ItemInstanceId](const FNotoEquippedItem& EquippedItem)
		{
			return EquippedItem.ItemInstanceId == ItemInstanceId;
		});
		Items.RemoveAll([ItemInstanceId](const FNotoItemInstance& ExistingItem)
		{
			return ExistingItem.InstanceId == ItemInstanceId;
		});

		if (RemovedEquipment > 0)
		{
			if (!FindEquipment(ActiveSlot))
			{
				ActiveSlot = ENotoEquipmentSlot::None;
			}
			MarkEquipmentDirty();
		}
	}

	MarkInventoryDirty();
}

void UNotoInventoryComponent::MarkInventoryDirty()
{
	bInventoryDirty = true;
	if (MutationDepth == 0)
	{
		FlushMutationNotifications();
	}
}

void UNotoInventoryComponent::MarkEquipmentDirty()
{
	bEquipmentDirty = true;
	if (MutationDepth == 0)
	{
		FlushMutationNotifications();
	}
}

void UNotoInventoryComponent::FlushMutationNotifications()
{
	const bool bBroadcastInventory = bInventoryDirty;
	const bool bBroadcastEquipment = bEquipmentDirty;
	bInventoryDirty = false;
	bEquipmentDirty = false;

	if (bBroadcastInventory)
	{
		OnInventoryChanged.Broadcast();
	}
	if (bBroadcastEquipment)
	{
		OnEquipmentChanged.Broadcast();
	}
}
