// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NotoInventoryTypes.h"
#include "NotoInventoryComponent.generated.h"

class UNotoItemDefinition;
class ANotoItemPickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotoInventoryChanged);

/** UI-independent owner of a player's carried items and logical equipment slots. */
UCLASS(BlueprintType)
class NOTOMORROWGAME_API UNotoInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Adds an item without equipping it. LoadedAmmo below zero initializes only magazine items as full. */
	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool AddItem(UNotoItemDefinition* Definition, int32 Quantity, int32 LoadedAmmo, FGuid& OutItemInstanceId);

	/** Adds an item and equips weapon/tool pickups. Duplicate magazine weapons transfer their loaded rounds instead. */
	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool CollectItem(UNotoItemDefinition* Definition, int32 Quantity, int32 LoadedAmmo, FGuid& OutItemInstanceId);

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool CanCollectItem(const UNotoItemDefinition* Definition, int32 Quantity, int32 LoadedAmmo) const;

	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool EquipItem(FGuid ItemInstanceId, ENotoEquipmentSlot Slot, bool bDropReplacedItem = true);

	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool UnequipItem(ENotoEquipmentSlot Slot);

	/** Selects which equipped slot is currently represented in the player's hands. */
	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool SetActiveSlot(ENotoEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool DropItem(FGuid ItemInstanceId, int32 Quantity);

	bool DropItemAt(FGuid ItemInstanceId, int32 Quantity, const FTransform& DropTransform);

	UFUNCTION(BlueprintCallable, Category = "Noto|Inventory")
	bool SetLoadedAmmo(FGuid ItemInstanceId, int32 LoadedAmmo);

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool GetItem(FGuid ItemInstanceId, FNotoItemInstance& OutItem) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool GetEquippedItem(ENotoEquipmentSlot Slot, FNotoItemInstance& OutItem) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool GetActiveItem(FNotoItemInstance& OutItem) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	ENotoEquipmentSlot GetActiveSlot() const { return ActiveSlot; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	int32 GetTotalQuantity(const UNotoItemDefinition* Definition) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool HasItemWithTag(FGameplayTag ItemTag, int32 MinimumQuantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	TArray<FNotoItemInstance> GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	TArray<FNotoEquippedItem> GetEquipment() const { return Equipment; }

	const TArray<FNotoItemInstance>& GetItemsView() const { return Items; }
	const TArray<FNotoEquippedItem>& GetEquipmentView() const { return Equipment; }

	UPROPERTY(BlueprintAssignable, Category = "Noto|Inventory")
	FNotoInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Noto|Inventory")
	FNotoInventoryChanged OnEquipmentChanged;

private:
	enum class ECollectionAction : uint8
	{
		AddOnly,
		TransferWeaponAmmo,
		FillEquippedStack,
		Equip,
		Replace
	};

	struct FCollectionPlan
	{
		ECollectionAction Action = ECollectionAction::AddOnly;
		ENotoEquipmentSlot Slot = ENotoEquipmentSlot::None;
		FGuid PreferredItemInstanceId;
		FGuid ReplacedItemInstanceId;
	};

	struct FScopedMutation
	{
		explicit FScopedMutation(UNotoInventoryComponent& InInventory);
		~FScopedMutation();

		UNotoInventoryComponent& Inventory;
	};

	bool AddItemInternal(UNotoItemDefinition* Definition, int32 Quantity, int32 LoadedAmmo,
	                     FGuid PreferredItemInstanceId, FGuid& OutItemInstanceId);
	bool BuildCollectionPlan(const UNotoItemDefinition& Definition, int32 Quantity, int32 LoadedAmmo,
	                         FCollectionPlan& OutPlan) const;
	static int32 ResolveLoadedAmmo(const UNotoItemDefinition& Definition, int32 LoadedAmmo);
	bool CanDropItem(FGuid ItemInstanceId, int32 Quantity) const;
	bool IsEquipmentItem(const UNotoItemDefinition& Definition) const;
	FNotoItemInstance* FindItem(FGuid ItemInstanceId);
	const FNotoItemInstance* FindItem(FGuid ItemInstanceId) const;
	FNotoEquippedItem* FindEquipment(ENotoEquipmentSlot Slot);
	const FNotoEquippedItem* FindEquipment(ENotoEquipmentSlot Slot) const;
	bool IsCompatibleSlot(const UNotoItemDefinition& Definition, ENotoEquipmentSlot Slot) const;
	bool TryGetDefaultDropTransform(FTransform& OutDropTransform) const;
	void RemoveItemQuantity(FGuid ItemInstanceId, int32 Quantity);
	void MarkInventoryDirty();
	void MarkEquipmentDirty();
	void FlushMutationNotifications();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category = "Noto|Inventory",
		Meta = (AllowPrivateAccess = "true"))
	TArray<FNotoItemInstance> Items;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category = "Noto|Inventory",
		Meta = (AllowPrivateAccess = "true"))
	TArray<FNotoEquippedItem> Equipment;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category = "Noto|Inventory",
		Meta = (AllowPrivateAccess = "true"))
	ENotoEquipmentSlot ActiveSlot = ENotoEquipmentSlot::None;

	UPROPERTY(EditDefaultsOnly, Category = "Noto|Inventory")
	TSubclassOf<ANotoItemPickup> DroppedPickupClass;

	int32 MutationDepth = 0;
	bool bInventoryDirty = false;
	bool bEquipmentDirty = false;
};
