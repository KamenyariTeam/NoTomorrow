// © 2025 Kamenyari. All rights reserved.

#include "Interaction/NotoTestPickup.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ANotoTestPickup::ANotoTestPickup()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Visual->SetStaticMesh(CubeMesh.Object);
		Visual->SetWorldScale3D(FVector(0.35f));
		Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ANotoTestPickup::Interact_Implementation(APawn* Interactor)
{
	Destroy();
}
