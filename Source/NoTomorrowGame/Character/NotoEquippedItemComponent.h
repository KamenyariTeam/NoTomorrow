// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NotoEquippedItemComponent.generated.h"

class ANotoEquippedItemActor;
class UNotoInventoryComponent;
class UNotoItemDefinition;
class UNotoHeldItemAnimationProfile;

/** No-tick visual and animation presentation for the item currently selected in the player's hands. */
UCLASS(BlueprintType, ClassGroup = (Noto), Meta = (BlueprintSpawnableComponent))
class NOTOMORROWGAME_API UNotoEquippedItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNotoEquippedItemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeWithInventory(UNotoInventoryComponent* InInventory);
	void UninitializeFromInventory();

	/** Plays the animation pair configured for ActionTag on the active item definition. */
	UFUNCTION(BlueprintCallable, Category = "Noto|Equipment", Meta = (Categories = "Item.Action"))
	bool PlayAction(FGameplayTag ActionTag);

	UFUNCTION(BlueprintPure, Category = "Noto|Equipment")
	FGuid GetDisplayedItemInstanceId() const { return DisplayedItemInstanceId; }

	UFUNCTION(BlueprintPure, Category = "Noto|Equipment")
	ANotoEquippedItemActor* GetDisplayedActor() const { return DisplayedActor; }

	UFUNCTION(BlueprintPure, Category = "Noto|Equipment")
	UNotoItemDefinition* GetDisplayedItemDefinition() const { return DisplayedDefinition; }

	UFUNCTION(BlueprintPure, Category = "Noto|Equipment|Animation")
	UNotoHeldItemAnimationProfile* GetDisplayedHeldItemAnimationProfile() const;

	/** Tests the active item's authored tags without coupling animation code to its presentation actor. */
	UFUNCTION(BlueprintPure, Category = "Noto|Equipment", Meta = (Categories = "Item"))
	bool HasDisplayedItemTag(FGameplayTag ItemTag, bool bExactMatch = false) const;

	virtual void OnUnregister() override;

private:
	UFUNCTION()
	void RefreshEquippedItem();

	void ClearEquippedItem();

	TWeakObjectPtr<UNotoInventoryComponent> Inventory;

	UPROPERTY(Transient)
	TObjectPtr<UNotoItemDefinition> DisplayedDefinition;

	UPROPERTY(Transient)
	TObjectPtr<ANotoEquippedItemActor> DisplayedActor;

	FGuid DisplayedItemInstanceId;
};
