// © 2025 Kamenyari. All rights reserved.

#include "Character/NotoEquippedItemComponent.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Character/NotoEquippedItemActor.h"
#include "Development/NotoGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Inventory/NotoInventoryComponent.h"
#include "Inventory/NotoItemDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoEquippedItemComponent)

UNotoEquippedItemComponent::UNotoEquippedItemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNotoEquippedItemComponent::InitializeWithInventory(UNotoInventoryComponent* InInventory)
{
	if (Inventory.Get() != InInventory)
	{
		UninitializeFromInventory();
		Inventory = InInventory;
		if (InInventory)
		{
			InInventory->OnEquipmentChanged.AddUniqueDynamic(this, &ThisClass::RefreshEquippedItem);
		}
	}
	RefreshEquippedItem();
}

void UNotoEquippedItemComponent::UninitializeFromInventory()
{
	if (UNotoInventoryComponent* BoundInventory = Inventory.Get())
	{
		BoundInventory->OnEquipmentChanged.RemoveDynamic(this, &ThisClass::RefreshEquippedItem);
	}
	Inventory.Reset();
	ClearEquippedItem();
}

bool UNotoEquippedItemComponent::PlayAction(FGameplayTag ActionTag)
{
	if (!DisplayedDefinition || !ActionTag.IsValid())
	{
		return false;
	}

	const FNotoEquippedItemAnimation* Animation = DisplayedDefinition->FindEquippedAnimation(ActionTag);
	if (!Animation)
	{
		return false;
	}

	bool bPlayed = false;
	if (Animation->CharacterMontage)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			bPlayed |= Character->PlayAnimMontage(Animation->CharacterMontage) > 0.0f;
		}
	}
	if (DisplayedActor)
	{
		bPlayed |= DisplayedActor->PlayItemAction(ActionTag, Animation->ItemMontage);
	}
	return bPlayed;
}

void UNotoEquippedItemComponent::OnUnregister()
{
	UninitializeFromInventory();
	Super::OnUnregister();
}

void UNotoEquippedItemComponent::RefreshEquippedItem()
{
	FNotoItemInstance Item;
	if (GetNetMode() == NM_DedicatedServer || !Inventory.IsValid()
		|| !Inventory->GetActiveItem(Item) || !Item.Definition || !Item.Definition->GetEquippedActorClass())
	{
		ClearEquippedItem();
		return;
	}

	if (DisplayedItemInstanceId == Item.InstanceId
		&& DisplayedDefinition == Item.Definition
		&& IsValid(DisplayedActor)
		&& DisplayedActor->GetClass() == Item.Definition->GetEquippedActorClass())
	{
		return;
	}

	ClearEquippedItem();
	DisplayedItemInstanceId = Item.InstanceId;
	DisplayedDefinition = Item.Definition;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		ClearEquippedItem();
		return;
	}

	DisplayedActor = World->SpawnActorDeferred<ANotoEquippedItemActor>(DisplayedDefinition->GetEquippedActorClass(), FTransform::Identity, Character, Character,
	                                                                   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!DisplayedActor)
	{
		ClearEquippedItem();
		return;
	}
	DisplayedActor->InitializeFromItem(DisplayedDefinition, DisplayedItemInstanceId);
	DisplayedActor->FinishSpawning(FTransform::Identity, true);
	DisplayedActor->SetActorRelativeTransform(DisplayedDefinition->GetEquippedRelativeTransform());
	USkeletalMeshComponent* AttachmentMesh = Character->GetMesh();
	if (!AttachmentMesh)
	{
		ClearEquippedItem();
		return;
	}

	DisplayedActor->AttachToComponent(AttachmentMesh, FAttachmentTransformRules::KeepRelativeTransform, DisplayedDefinition->GetEquippedSocketName());
	PlayAction(NotoGameplayTags::ItemAction_Equip);
}

void UNotoEquippedItemComponent::ClearEquippedItem()
{
	DisplayedItemInstanceId.Invalidate();
	DisplayedDefinition = nullptr;
	if (DisplayedActor)
	{
		DisplayedActor->Destroy();
		DisplayedActor = nullptr;
	}
}
