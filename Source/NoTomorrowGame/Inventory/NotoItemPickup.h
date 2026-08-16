// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Interaction/NotoInteractableActor.h"
#include "NotoItemPickup.generated.h"

class UNotoItemDefinition;

/** Generic world representation that transfers authored item data into a player's inventory. */
UCLASS(Blueprintable)
class NOTOMORROWGAME_API ANotoItemPickup : public ANotoInteractableActor
{
	GENERATED_BODY()

public:
	ANotoItemPickup();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool CanInteract_Implementation(APawn* Interactor) const override;
	virtual void Interact_Implementation(APawn* Interactor, const FNotoInteractionRequest& Request) override;

	void InitializePickup(UNotoItemDefinition* InDefinition, int32 InQuantity, int32 InLoadedAmmo);

private:
	bool TryPickUp(APawn* Interactor, bool bMakeCollectedItemActive);
	void RefreshVisual();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noto|Inventory", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNotoItemDefinition> ItemDefinition;

	/** Must be one for item definitions whose effective maximum stack size is one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noto|Inventory",
		Meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 Quantity = 1;

	/** Below zero initializes a magazine weapon with a full magazine; other items use zero. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noto|Inventory", Meta = (AllowPrivateAccess = "true"))
	int32 LoadedAmmo = -1;
};
