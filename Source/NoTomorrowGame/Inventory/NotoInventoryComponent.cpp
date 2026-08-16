// © 2025 Kamenyari. All rights reserved.

#include "Inventory/NotoInventoryComponent.h"

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

	const int32 InitialLoadedAmmo = ResolveLoadedAmmo(*Definition, LoadedAmmo);

	while (RemainingQuantity > 0)
	{
		FNotoItemInstance& Item = Items.AddDefaulted_GetRef();
		Item.InstanceId = FGuid::NewGuid();
		Item.DefinitionId = DefinitionId;
		Item.Definition = Definition;
		Item.Quantity = FMath::Min(RemainingQuantity, MaxStackSize);
		Item.LoadedAmmo = InitialLoadedAmmo;
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
	FGuid& OutItemInstanceId)
{
	OutItemInstanceId.Invalidate();
	FCollectionPlan Plan;
	if (!Definition || !BuildCollectionPlan(*Definition, Quantity, LoadedAmmo, Plan))
	{
		return false;
	}

	FScopedMutation Mutation(*this);
	if (Plan.Action == ECollectionAction::TransferWeaponAmmo)
	{
		FNotoItemInstance* EquippedWeapon = FindItem(Plan.PreferredItemInstanceId);
		check(EquippedWeapon && EquippedWeapon->DefinitionId == Definition->GetPrimaryAssetId() && Definition->
			UsesMagazine());

		OutItemInstanceId = EquippedWeapon->InstanceId;
		EquippedWeapon->LoadedAmmo = FMath::Min(
			EquippedWeapon->LoadedAmmo + ResolveLoadedAmmo(*Definition, LoadedAmmo),
			Definition->GetMagazineCapacity());
		MarkInventoryDirty();
		return SetActiveSlot(Plan.Slot);
	}

	if (Plan.Action == ECollectionAction::Replace)
	{
		const FNotoItemInstance* ReplacedItem = FindItem(Plan.ReplacedItemInstanceId);
		check(ReplacedItem);
		if (!DropItem(ReplacedItem->InstanceId, ReplacedItem->Quantity))
		{
			return false;
		}
	}

	const bool bAdded = AddItemInternal(Definition, Quantity, LoadedAmmo, Plan.PreferredItemInstanceId,
	                                    OutItemInstanceId);
	check(bAdded && OutItemInstanceId.IsValid());

	switch (Plan.Action)
	{
	case ECollectionAction::FillEquippedStack:
		check(OutItemInstanceId == Plan.PreferredItemInstanceId);
		return SetActiveSlot(Plan.Slot);
	case ECollectionAction::Equip:
	case ECollectionAction::Replace:
		return EquipItem(OutItemInstanceId, Plan.Slot, false);
	case ECollectionAction::AddOnly:
	default:
		return true;
	}
}

bool UNotoInventoryComponent::CanCollectItem(const UNotoItemDefinition* Definition, int32 Quantity,
                                             int32 LoadedAmmo) const
{
	FCollectionPlan Plan;
	return Definition && BuildCollectionPlan(*Definition, Quantity, LoadedAmmo, Plan);
}

bool UNotoInventoryComponent::BuildCollectionPlan(
	const UNotoItemDefinition& Definition,
	int32 Quantity,
	int32 LoadedAmmo,
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

	if (Definition.UsesMagazine() && Quantity == 1)
	{
		for (const ENotoEquipmentSlot Slot : CompatibleSlots)
		{
			const FNotoEquippedItem* EquippedItem = FindEquipment(Slot);
			const FNotoItemInstance* Item = EquippedItem ? FindItem(EquippedItem->ItemInstanceId) : nullptr;
			if (Item && Item->DefinitionId == DefinitionId)
			{
				if (ResolveLoadedAmmo(Definition, LoadedAmmo) <= 0 || Item->LoadedAmmo >= Definition.
					GetMagazineCapacity())
				{
					return false;
				}

				OutPlan.Action = ECollectionAction::TransferWeaponAmmo;
				OutPlan.Slot = Slot;
				OutPlan.PreferredItemInstanceId = Item->InstanceId;
				return true;
			}
		}
	}

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

	auto PlanReplacement = [this, &OutPlan](ENotoEquipmentSlot Slot)
	{
		const FNotoEquippedItem* EquippedItem = FindEquipment(Slot);
		const FNotoItemInstance* Item = EquippedItem ? FindItem(EquippedItem->ItemInstanceId) : nullptr;
		if (!Item || !CanDropItem(Item->InstanceId, Item->Quantity))
		{
			return false;
		}

		OutPlan.Action = ECollectionAction::Replace;
		OutPlan.Slot = Slot;
		OutPlan.ReplacedItemInstanceId = Item->InstanceId;
		return true;
	};

	if (Definition.GetItemType() != ENotoItemType::Tool)
	{
		return PlanReplacement(CompatibleSlots[0]);
	}

	if (ActiveSlot >= ENotoEquipmentSlot::Tool1
		&& ActiveSlot <= ENotoEquipmentSlot::Tool5
		&& PlanReplacement(ActiveSlot))
	{
		return true;
	}

	for (const ENotoEquipmentSlot Slot : CompatibleSlots)
	{
		if (Slot != ActiveSlot && PlanReplacement(Slot))
		{
			return true;
		}
	}
	return false;
}

int32 UNotoInventoryComponent::ResolveLoadedAmmo(const UNotoItemDefinition& Definition, int32 LoadedAmmo)
{
	return Definition.UsesMagazine()
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

	Pickup->InitializePickup(Item->Definition, Quantity, Item->LoadedAmmo);
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
	if (!Item
		|| !Item->Definition
		|| !Item->Definition->UsesMagazine()
		|| Item->Quantity != 1
		|| Item->Definition->GetMaxStackSize() != 1
		|| LoadedAmmo < 0
		|| LoadedAmmo > Item->Definition->GetMagazineCapacity())
	{
		return false;
	}

	if (Item->LoadedAmmo != LoadedAmmo)
	{
		Item->LoadedAmmo = LoadedAmmo;
		MarkInventoryDirty();
	}
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

bool UNotoInventoryComponent::IsEquipmentItem(const UNotoItemDefinition& Definition) const
{
	return Definition.GetItemType() == ENotoItemType::MainWeapon
		|| Definition.GetItemType() == ENotoItemType::SecondaryWeapon
		|| Definition.GetItemType() == ENotoItemType::Tool;
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

	OutDropTransform = FTransform(Pawn->GetActorRotation(),
	                              Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 100.0f);
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
