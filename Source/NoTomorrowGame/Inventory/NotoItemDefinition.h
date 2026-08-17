// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NotoItemDefinition.generated.h"

class UStaticMesh;

/** Broad gameplay role of an item. Specific weapon and tool families belong in ItemTags. */
UENUM(BlueprintType)
enum class ENotoItemType : uint8
{
	MainWeapon,
	SecondaryWeapon,
	Tool,
	QuestItem,
	Lore
};

/** Immutable authored data shared by every runtime instance of an item. */
UCLASS(BlueprintType)
class NOTOMORROWGAME_API UNotoItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	ENotoItemType GetItemType() const { return ItemType; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	FGameplayTagContainer GetItemTags() const { return ItemTags; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	int32 GetMaxStackSize() const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool IsDroppable() const { return bDroppable; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	int32 GetMagazineCapacity() const { return FMath::Max(0, MagazineCapacity); }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool UsesMagazine() const;

	UStaticMesh* GetWorldMesh() const { return WorldMesh; }

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", Meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", Meta = (AllowPrivateAccess = "true"))
	ENotoItemType ItemType = ENotoItemType::MainWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", Meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item",
		Meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", Meta = (AllowPrivateAccess = "true"))
	bool bDroppable = true;

	/** Zero represents a weapon without a magazine. Magazine ammunition lives on the weapon instance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", Meta = (AllowPrivateAccess = "true", ClampMin = "0", EditCondition = "ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon", EditConditionHides))
	int32 MagazineCapacity = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> WorldMesh;
};
