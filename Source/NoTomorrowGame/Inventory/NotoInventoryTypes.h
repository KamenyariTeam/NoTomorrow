// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManagerTypes.h"
#include "NotoInventoryTypes.generated.h"

class UNotoItemDefinition;

UENUM(BlueprintType)
enum class ENotoEquipmentSlot : uint8
{
	// These values are persisted. Append future slots instead of reordering them.
	None = 0,
	MainWeapon = 1,
	SecondaryWeapon = 2,
	Tool1 = 3,
	Tool2 = 4,
	Tool3 = 5,
	Tool4 = 6,
	Tool5 = 7
};

/** Mutable, save-friendly state for one item stack or unique item. */
USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoItemInstance
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	FGuid InstanceId;

	/** Stable authored identity used when this struct is persisted. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	FPrimaryAssetId DefinitionId;

	/** Runtime resolved asset; a future load pass can resolve it from DefinitionId. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNotoItemDefinition> Definition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int32 Quantity = 0;

	/** Per-instance magazine ammunition. There are no standalone ammunition items. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int32 LoadedAmmo = 0;
};

/** Slot state refers to an item instance instead of duplicating its mutable data. */
USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoEquippedItem
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	ENotoEquipmentSlot Slot = ENotoEquipmentSlot::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	FGuid ItemInstanceId;
};
