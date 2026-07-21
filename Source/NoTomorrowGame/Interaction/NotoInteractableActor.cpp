// © 2025 Kamenyari. All rights reserved.

#include "Interaction/NotoInteractableActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

namespace
{
	constexpr int32 InteractionStencilValue = 1;
}

ANotoInteractableActor::ANotoInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	SetRootComponent(Visual);

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Visual);
	InteractionVolume->InitSphereRadius(100.0f);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

bool ANotoInteractableActor::CanInteract_Implementation(APawn* Interactor) const
{
	return IsValid(Interactor);
}

void ANotoInteractableActor::SetInteractionHighlighted_Implementation(bool bHighlighted)
{
	Visual->SetCustomDepthStencilValue(InteractionStencilValue);
	Visual->SetRenderCustomDepth(bHighlighted);
}
