// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/NotoInventoryTypes.h"
#include "UObject/Object.h"
#include "NotoInventoryTestListener.generated.h"

class UNotoInventoryComponent;

struct FNotoInventoryTestObservation
{
	int32 Quantity = INDEX_NONE;
	int32 ItemCount = INDEX_NONE;
	int32 EquipmentCount = INDEX_NONE;
	ENotoEquipmentSlot ActiveSlot = ENotoEquipmentSlot::None;
};

UCLASS()
class UNotoInventoryTestListener : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UNotoInventoryComponent* InInventory);
	void Reset();

	UFUNCTION()
	void HandleInventoryChanged();

	UFUNCTION()
	void HandleEquipmentChanged();

	int32 InventoryChangedCount = 0;
	int32 EquipmentChangedCount = 0;
	FNotoInventoryTestObservation InventoryObservation;
	FNotoInventoryTestObservation EquipmentObservation;
	TArray<FName> CallbackOrder;

private:
	void RecordObservation(FNotoInventoryTestObservation& Observation);

	UPROPERTY()
	TObjectPtr<UNotoInventoryComponent> Inventory;
};
