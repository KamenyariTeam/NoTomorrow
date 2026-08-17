// © 2025 Kamenyari. All rights reserved.

#include "Inventory/NotoItemPickup.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Inventory/NotoInventoryComponent.h"
#include "Inventory/NotoItemDefinition.h"
#include "Player/NotoPlayerState.h"

ANotoItemPickup::ANotoItemPickup()
{
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANotoItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisual();
}

bool ANotoItemPickup::CanInteract_Implementation(APawn* Interactor) const
{
	const ANotoPlayerState* PlayerState = Interactor ? Interactor->GetPlayerState<ANotoPlayerState>() : nullptr;
	const UNotoInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	return bHasRuntimeItemInstance
		       ? Inventory->CanCollectItemInstance(RuntimeItemInstance)
		       : Inventory->CanCollectItem(ItemDefinition, Quantity, LoadedAmmo);
}

void ANotoItemPickup::Interact_Implementation(APawn* Interactor, const FNotoInteractionRequest& Request)
{
	TryPickUp(Interactor, !Request.bUseAlternateInteraction);
}

bool ANotoItemPickup::TryPickUp(APawn* Interactor, bool bMakeCollectedItemActive)
{
	ANotoPlayerState* PlayerState = Interactor ? Interactor->GetPlayerState<ANotoPlayerState>() : nullptr;
	UNotoInventoryComponent* Inventory = PlayerState ? PlayerState->GetInventoryComponent() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	FGuid ItemInstanceId;
	int32 RemainingLoadedAmmo = 0;
	const bool bCollected = bHasRuntimeItemInstance
		                        ? Inventory->CollectItemInstance(
			                        RuntimeItemInstance,
			                        ItemInstanceId,
			                        bMakeCollectedItemActive)
		                        : Inventory->CollectItem(
			                        ItemDefinition,
			                        Quantity,
			                        LoadedAmmo,
			                        ItemInstanceId,
			                        RemainingLoadedAmmo,
			                        bMakeCollectedItemActive);
	if (bCollected)
	{
		Destroy();
		return true;
	}
	return false;
}

void ANotoItemPickup::InitializePickup(
	UNotoItemDefinition* InDefinition,
	int32 InQuantity,
	int32 InLoadedAmmo)
{
	check(InDefinition && InQuantity > 0);
	ItemDefinition = InDefinition;
	Quantity = InQuantity;
	LoadedAmmo = InLoadedAmmo;
	bHasRuntimeItemInstance = false;
	RuntimeItemInstance = FNotoItemInstance();
	RefreshVisual();
}

void ANotoItemPickup::InitializePickup(const FNotoItemInstance& InItemInstance)
{
	check(InItemInstance.Definition && InItemInstance.Quantity > 0);
	RuntimeItemInstance = InItemInstance;
	bHasRuntimeItemInstance = true;
	ItemDefinition = InItemInstance.Definition;
	Quantity = InItemInstance.Quantity;
	LoadedAmmo = InItemInstance.LoadedAmmo;
	RefreshVisual();
}

void ANotoItemPickup::RefreshVisual()
{
	Visual->SetStaticMesh(ItemDefinition ? ItemDefinition->GetWorldMesh() : nullptr);
}
