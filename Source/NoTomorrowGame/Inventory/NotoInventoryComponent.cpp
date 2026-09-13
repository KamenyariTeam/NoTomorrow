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
	bool bMakeCollectedItemActive)
{
	OutItemInstanceId.Invalidate();
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

bool UNotoInventoryComponent::CanCollectItem(const UNotoItemDefinition* Definition, int32 Quantity) const
{
	FCollectionPlan Plan;
	return Definition && BuildCollectionPlan(*Definition, Quantity, Plan);
}

bool UNotoInventoryComponent::CanCollectItemInstance(const FNotoItemInstance& ItemInstance) const
{
	FCollectionPlan Plan;
	return IsItemStateValid(ItemInstance)
		&& !IsInstanceIdInUse(ItemInstance.InstanceId)
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
	if (Definition.UsesAmmunition())
	{
		Item.LoadedAmmo = ResolveLoadedAmmo(Definition, LoadedAmmo);
	}
}

bool UNotoInventoryComponent::IsItemStateValid(const FNotoItemInstance& Item)
{
	if (!Item.InstanceId.IsValid() || !Item.Definition || Item.DefinitionId != Item.Definition->GetPrimaryAssetId()
		|| Item.Quantity <= 0 || (Item.Definition->GetMaxStackSize() == 1 && Item.Quantity != 1))
	{
		return false;
	}

	if (Item.Definition->UsesAmmunition())
	{
		return Item.LoadedAmmo >= 0 && Item.LoadedAmmo <= Item.Definition->GetAmmoCapacity();
	}
	return Item.LoadedAmmo == 0;
}

int32 UNotoInventoryComponent::ResolveLoadedAmmo(const UNotoItemDefinition& Definition, int32 LoadedAmmo)
{
	return Definition.UsesAmmunition()
		       ? FMath::Clamp(LoadedAmmo < 0 ? Definition.GetAmmoCapacity() : LoadedAmmo, 0,
		                      Definition.GetAmmoCapacity())
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
	if (Item->Definition->UsesAmmunition())
	{
		MutableAmmo = &Item->LoadedAmmo;
		Capacity = Item->Definition->GetAmmoCapacity();
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
	if (!IsItemStateValid(ItemInstance) || IsInstanceIdInUse(ItemInstance.InstanceId))
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

	if (Item->LoadedAmmo < Amount)
	{
		return false;
	}

	Item->LoadedAmmo -= Amount;
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
	UNotoItemDefinition* AmmunitionDefinition = WeaponDefinition.GetAmmunitionDefinition();
	if (!WeaponDefinition.UsesAmmunition() || !AmmunitionDefinition || !AmmunitionDefinition->IsAmmunition()
		|| AmmunitionDefinition->GetMaxStackSize() <= 1)
	{
		return false;
	}

	const int32 MissingRounds = WeaponDefinition.GetAmmoCapacity() - Items[WeaponIndex].LoadedAmmo;
	if (MissingRounds <= 0)
	{
		return false;
	}

	const int32 MaximumReload = WeaponDefinition.GetAmmoFeedType() == ENotoAmmoFeedType::Internal
		                            ? 1
		                            : MissingRounds;
	int32 RemainingToLoad = FMath::Min(MissingRounds, MaximumReload);
	for (int32 Index = Items.Num() - 1; Index >= 0 && RemainingToLoad > 0; --Index)
	{
		FNotoItemInstance& Candidate = Items[Index];
		if (Candidate.Definition != AmmunitionDefinition || Candidate.Quantity <= 0)
		{
			continue;
		}

		const int32 ConsumedRounds = FMath::Min(Candidate.Quantity, RemainingToLoad);
		Candidate.Quantity -= ConsumedRounds;
		RemainingToLoad -= ConsumedRounds;
		if (Candidate.Quantity == 0)
		{
			Items.RemoveAtSwap(Index, EAllowShrinking::No);
		}
	}

	OutReloadedRounds = FMath::Min(MissingRounds, MaximumReload) - RemainingToLoad;
	if (OutReloadedRounds <= 0)
	{
		return false;
	}
	FNotoItemInstance* Weapon = FindItem(ItemInstanceId);
	check(Weapon);
	Weapon->LoadedAmmo += OutReloadedRounds;
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

		const int32 NormalizedAmmo = Item.Definition->UsesAmmunition()
			                             ? FMath::Clamp(Item.LoadedAmmo, 0, Item.Definition->GetAmmoCapacity())
			                             : 0;
		if (NormalizedAmmo != Item.LoadedAmmo)
		{
			Item.LoadedAmmo = NormalizedAmmo;
			bAnyDefinitionChanged = true;
		}
	}

	if (bAnyDefinitionChanged)
	{
		MarkInventoryDirty();
		MarkEquipmentDirty();
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
		return Item.InstanceId == InstanceId;
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
