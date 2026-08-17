// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
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
	Lore,
	Magazine,
	Ammunition
};

UENUM(BlueprintType)
enum class ENotoAmmoFeedType : uint8
{
	None,
	DetachableMagazine,
	Internal
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
	bool IsMagazine() const { return ItemType == ENotoItemType::Magazine; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool IsLooseAmmunition() const { return ItemType == ENotoItemType::Ammunition; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	ENotoAmmoFeedType GetAmmoFeedType() const { return AmmoFeedType; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	FGameplayTag GetAmmoFamily() const { return AmmoFamily; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	int32 GetMagazineCapacity() const { return IsMagazine() ? FMath::Max(0, MagazineCapacity) : 0; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	int32 GetInternalCapacity() const { return UsesInternalAmmo() ? FMath::Max(0, InternalCapacity) : 0; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	UNotoItemDefinition* GetStandardMagazineDefinition() const { return StandardMagazineDefinition; }

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool UsesMagazine() const;

	UFUNCTION(BlueprintPure, Category = "Noto|Inventory")
	bool UsesInternalAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	bool IsFirearm() const { return ItemType == ENotoItemType::MainWeapon || bFirearm; }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	float GetFirearmDamage() const { return FMath::Max(0.0f, FirearmDamage); }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	float GetFirearmRange() const { return FMath::Max(0.0f, FirearmRange); }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	float GetFirearmTraceHeight() const { return FirearmTraceHeight; }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	float GetFireInterval() const { return FMath::Max(0.0f, FireInterval); }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	int32 GetAmmoPerShot() const { return FMath::Max(1, AmmoPerShot); }

	UFUNCTION(BlueprintPure, Category = "Noto|Weapon")
	float GetGunfireNoiseRange() const { return FMath::Max(0.0f, GunfireNoiseRange); }

	ECollisionChannel GetFirearmTraceChannel() const { return FirearmTraceChannel.GetValue(); }

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition",
		Meta = (AllowPrivateAccess = "true", EditCondition = "ItemType == ENotoItemType::MainWeapon || ItemType == ENotoItemType::SecondaryWeapon", EditConditionHides))
	ENotoAmmoFeedType AmmoFeedType = ENotoAmmoFeedType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition",
		Meta = (AllowPrivateAccess = "true", Categories = "Ammo", EditCondition = "AmmoFeedType != ENotoAmmoFeedType::None || ItemType == ENotoItemType::Magazine || ItemType == ENotoItemType::Ammunition", EditConditionHides))
	FGameplayTag AmmoFamily;

	/** Standard magazine inserted by an authored weapon pickup and used for legacy-save migration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition",
		Meta = (AllowPrivateAccess = "true", EditCondition = "AmmoFeedType == ENotoAmmoFeedType::DetachableMagazine", EditConditionHides))
	TObjectPtr<UNotoItemDefinition> StandardMagazineDefinition;

	/** Capacity belongs to a physical magazine definition, never a detachable-magazine weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition",
		Meta = (AllowPrivateAccess = "true", ClampMin = "0", EditCondition = "ItemType == ENotoItemType::Magazine", EditConditionHides))
	int32 MagazineCapacity = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammunition",
		Meta = (AllowPrivateAccess = "true", ClampMin = "0", EditCondition = "AmmoFeedType == ENotoAmmoFeedType::Internal", EditConditionHides))
	int32 InternalCapacity = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", EditCondition = "AmmoFeedType != ENotoAmmoFeedType::None", EditConditionHides))
	bool bFirearm = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", ClampMin = "0.0", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	float FirearmDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	float FirearmRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", Units = "cm", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	float FirearmTraceHeight = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "s", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	float FireInterval = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", ClampMin = "1", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	int32 AmmoPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	float GunfireNoiseRange = 3000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Firearm",
		Meta = (AllowPrivateAccess = "true", EditCondition = "bFirearm || ItemType == ENotoItemType::MainWeapon", EditConditionHides))
	TEnumAsByte<ECollisionChannel> FirearmTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> WorldMesh;
};
