// © 2025 Kamenyari. All rights reserved.

#include "Player/NotoCheatManager.h"

#include "Engine/Engine.h"
#include "Inventory/NotoInventoryComponent.h"
#include "Inventory/NotoItemDefinition.h"
#include "Player/NotoPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoCheatManager)

DEFINE_LOG_CATEGORY_STATIC(LogNotoInventoryDebug, Log, All);

namespace
{
	constexpr uint64 InventoryDebugMessageKey = 0x4E6F746F496E76;
}

void UNotoCheatManager::InitCheatManager()
{
	Super::InitCheatManager();
	RefreshInventoryBinding();
}

void UNotoCheatManager::BeginDestroy()
{
	UnbindInventory();
	Super::BeginDestroy();
}

void UNotoCheatManager::HandlePlayerStateChanged()
{
	RefreshInventoryBinding();
}

void UNotoCheatManager::NotoInventoryDump() const
{
	const UNotoInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("No inventory is available."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InventoryDebugMessageKey, 5.0f, FColor::Yellow,
			                                 TEXT("Inventory: no PlayerState or inventory available."));
		}
		return;
	}

	FString Dump = FString::Printf(TEXT("Inventory: active=%s, %d stack(s), %d equipped"),
	                               *UEnum::GetValueAsString(Inventory->GetActiveSlot()),
	                               Inventory->GetItemsView().Num(),
	                               Inventory->GetEquipmentView().Num());
	UE_LOG(LogNotoInventoryDebug, Display, TEXT("%s"), *Dump);

	for (const FNotoItemInstance& Item : Inventory->GetItemsView())
	{
		ENotoEquipmentSlot EquippedSlot = ENotoEquipmentSlot::None;
		for (const FNotoEquippedItem& EquippedItem : Inventory->GetEquipmentView())
		{
			if (EquippedItem.ItemInstanceId == Item.InstanceId)
			{
				EquippedSlot = EquippedItem.Slot;
				break;
			}
		}

		const FString DefinitionName = Item.Definition
			                               ? Item.Definition->GetPrimaryAssetId().ToString()
			                               : Item.DefinitionId.ToString();
		const FString DisplayName = Item.Definition
			                            ? Item.Definition->GetDisplayName().ToString()
			                            : TEXT("<unresolved>");
		const FString ItemType = Item.Definition
			                         ? UEnum::GetValueAsString(Item.Definition->GetItemType())
			                         : TEXT("<unknown>");
		FString Line = FString::Printf(TEXT("%s \"%s\" type=%s id=%s quantity=%d loaded=%d"),
		                               *DefinitionName,
		                               *DisplayName,
		                               *ItemType,
		                               *Item.InstanceId.ToString(),
		                               Item.Quantity,
		                               Item.LoadedAmmo);
		if (EquippedSlot != ENotoEquipmentSlot::None)
		{
			Line += FString::Printf(TEXT(" equipped=%s"), *UEnum::GetValueAsString(EquippedSlot));
		}
		UE_LOG(LogNotoInventoryDebug, Display, TEXT("  %s"), *Line);
		Dump += LINE_TERMINATOR + Line;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(InventoryDebugMessageKey, 10.0f, FColor::White, Dump);
	}
}

void UNotoCheatManager::NotoInventoryDebug()
{
	bInventoryDebugEnabled = !bInventoryDebugEnabled;
	RefreshInventoryBinding();
	UE_LOG(LogNotoInventoryDebug, Display, TEXT("Automatic inventory dumps %s."),
	       bInventoryDebugEnabled ? TEXT("enabled") : TEXT("disabled"));
	if (bInventoryDebugEnabled)
	{
		NotoInventoryDump();
	}
}

void UNotoCheatManager::NotoInventoryGive(const FString& ItemDefinitionPath, int32 Quantity, int32 LoadedAmmo)
{
	UNotoInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("No inventory is available."));
		return;
	}
	if (ItemDefinitionPath.IsEmpty() || Quantity <= 0)
	{
		UE_LOG(LogNotoInventoryDebug, Warning,
		       TEXT("Usage: NotoInventoryGive <ItemDefinitionPath> [Quantity] [LoadedAmmo]"));
		return;
	}

	UNotoItemDefinition* Definition = LoadObject<UNotoItemDefinition>(nullptr, *ItemDefinitionPath);
	if (!Definition)
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("'%s' is not a valid item-definition asset."), *ItemDefinitionPath);
		return;
	}

	FGuid ItemInstanceId;
	int32 RemainingLoadedAmmo = 0;
	if (!Inventory->CollectItem(Definition, Quantity, LoadedAmmo, ItemInstanceId, RemainingLoadedAmmo))
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("Could not collect '%s' x%d."), *ItemDefinitionPath, Quantity);
		return;
	}

	UE_LOG(LogNotoInventoryDebug, Display, TEXT("Collected '%s' x%d as %s."),
	       *ItemDefinitionPath,
	       Quantity,
	       *ItemInstanceId.ToString());
	if (RemainingLoadedAmmo > 0)
	{
		UE_LOG(LogNotoInventoryDebug, Warning,
		       TEXT("%d loaded round(s) did not fit and were not granted by the cheat command."),
		       RemainingLoadedAmmo);
	}
}

void UNotoCheatManager::NotoInventoryDropActive(int32 Quantity)
{
	UNotoInventoryComponent* Inventory = GetInventoryComponent();
	FNotoItemInstance ActiveItem;
	if (!Inventory)
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("No inventory is available."));
		return;
	}
	if (Quantity <= 0 || !Inventory->GetActiveItem(ActiveItem))
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("No active item or invalid drop quantity %d."), Quantity);
		return;
	}
	if (!Inventory->DropItem(ActiveItem.InstanceId, Quantity))
	{
		UE_LOG(LogNotoInventoryDebug, Warning,
		       TEXT(
			       "Could not drop %d from the active item; check quantity, droppable state, pawn, world, and pickup class."
		       ), Quantity);
	}
}

void UNotoCheatManager::NotoInventorySetActiveSlot(int32 SlotValue)
{
	const UEnum* SlotEnum = StaticEnum<ENotoEquipmentSlot>();
	UNotoInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("No inventory is available."));
		return;
	}
	if (!SlotEnum || !SlotEnum->IsValidEnumValue(SlotValue))
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("%d is not a valid ENotoEquipmentSlot value."), SlotValue);
		return;
	}

	const ENotoEquipmentSlot Slot = static_cast<ENotoEquipmentSlot>(SlotValue);
	if (!Inventory->SetActiveSlot(Slot))
	{
		UE_LOG(LogNotoInventoryDebug, Warning, TEXT("Slot %s is not currently equipped."),
		       *UEnum::GetValueAsString(Slot));
	}
}

void UNotoCheatManager::HandleInventoryChanged()
{
	if (bInventoryDebugEnabled)
	{
		NotoInventoryDump();
	}
}

UNotoInventoryComponent* UNotoCheatManager::GetInventoryComponent() const
{
	const ANotoPlayerController* PlayerController = Cast<ANotoPlayerController>(GetPlayerController());
	return PlayerController ? PlayerController->GetInventoryComponent() : nullptr;
}

void UNotoCheatManager::RefreshInventoryBinding()
{
	UNotoInventoryComponent* Inventory = GetInventoryComponent();
	if (BoundInventory.Get() == Inventory && (!Inventory || bInventoryDebugEnabled))
	{
		return;
	}

	UnbindInventory();
	if (bInventoryDebugEnabled && Inventory)
	{
		Inventory->OnInventoryChanged.AddUniqueDynamic(this, &ThisClass::HandleInventoryChanged);
		Inventory->OnEquipmentChanged.AddUniqueDynamic(this, &ThisClass::HandleInventoryChanged);
		BoundInventory = Inventory;
	}
}

void UNotoCheatManager::UnbindInventory()
{
	if (BoundInventory.IsValid())
	{
		BoundInventory->OnInventoryChanged.RemoveDynamic(this, &ThisClass::HandleInventoryChanged);
		BoundInventory->OnEquipmentChanged.RemoveDynamic(this, &ThisClass::HandleInventoryChanged);
	}
	BoundInventory.Reset();
}
