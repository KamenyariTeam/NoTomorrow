// © 2025 Kamenyari. All rights reserved.

#include "Character/NotoEquippedItemActor.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoEquippedItemActor)

ANotoEquippedItemActor::ANotoEquippedItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);
	SetReplicates(false);
}

void ANotoEquippedItemActor::InitializeFromItem(UNotoItemDefinition* InDefinition, FGuid InItemInstanceId)
{
	ItemDefinition = InDefinition;
	ItemInstanceId = InItemInstanceId;
}

bool ANotoEquippedItemActor::PlayItemAction(FGameplayTag ActionTag, UAnimMontage* ItemMontage)
{
	ReceiveItemAction(ActionTag);
	if (ItemMontage)
	{
		if (USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(ItemMontage);
			}
		}
	}
	return true;
}
