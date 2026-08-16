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

	return Inventory->CanCollectItem(ItemDefinition, Quantity, LoadedAmmo);
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
	const bool bCollected = Inventory->CollectItem(
		ItemDefinition,
		Quantity,
		LoadedAmmo,
		ItemInstanceId,
		RemainingLoadedAmmo,
		bMakeCollectedItemActive);
	if (bCollected)
	{
		if (RemainingLoadedAmmo > 0)
		{
			LoadedAmmo = RemainingLoadedAmmo;
			return true;
		}

		Destroy();
		return true;
	}
	return false;
}

void ANotoItemPickup::InitializePickup(UNotoItemDefinition* InDefinition, int32 InQuantity, int32 InLoadedAmmo)
{
	check(InDefinition && InQuantity > 0);
	ItemDefinition = InDefinition;
	Quantity = InQuantity;
	LoadedAmmo = InLoadedAmmo;
	RefreshVisual();
}

void ANotoItemPickup::RefreshVisual()
{
	Visual->SetStaticMesh(ItemDefinition ? ItemDefinition->GetWorldMesh() : nullptr);
}
