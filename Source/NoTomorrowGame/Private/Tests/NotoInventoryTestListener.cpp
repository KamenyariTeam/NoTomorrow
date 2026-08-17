// © 2025 Kamenyari. All rights reserved.

#include "Tests/NotoInventoryTestListener.h"

#include "Inventory/NotoInventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoInventoryTestListener)

void UNotoInventoryTestListener::Initialize(UNotoInventoryComponent* InInventory)
{
	Inventory = InInventory;
}

void UNotoInventoryTestListener::Reset()
{
	InventoryChangedCount = 0;
	EquipmentChangedCount = 0;
	InventoryObservation = FNotoInventoryTestObservation();
	EquipmentObservation = FNotoInventoryTestObservation();
	CallbackOrder.Reset();
}

void UNotoInventoryTestListener::HandleInventoryChanged()
{
	++InventoryChangedCount;
	CallbackOrder.Add(TEXT("Inventory"));
	RecordObservation(InventoryObservation);
}

void UNotoInventoryTestListener::HandleEquipmentChanged()
{
	++EquipmentChangedCount;
	CallbackOrder.Add(TEXT("Equipment"));
	RecordObservation(EquipmentObservation);
}

void UNotoInventoryTestListener::RecordObservation(FNotoInventoryTestObservation& Observation)
{
	Observation.ItemCount = Inventory->GetItemsView().Num();
	Observation.EquipmentCount = Inventory->GetEquipmentView().Num();
	Observation.ActiveSlot = Inventory->GetActiveSlot();
	Observation.Quantity = 0;
	for (const FNotoItemInstance& Item : Inventory->GetItemsView())
	{
		Observation.Quantity += Item.Quantity;
	}
}
