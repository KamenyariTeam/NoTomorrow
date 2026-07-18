// © 2025 Kamenyari. All rights reserved.

#include "NotoCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "NotoPlayerPawnComponent.h"

ANotoCharacter::ANotoCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Disable ticking if not needed.
	PrimaryActorTick.bCanEverTick = false;

	// Set up capsule component collision settings.
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));

	// Set up mesh rotation if needed (adjust as appropriate).
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		MeshComp->SetCollisionProfileName(TEXT("PawnMesh"));
	}
	
	GameplayCameraComponent = CreateDefaultSubobject<UGameplayCameraComponent>(TEXT("CameraComponent"));
	GameplayCameraComponent->SetupAttachment(RootComponent);
	GameplayCameraComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void ANotoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UNotoPlayerPawnComponent* PlayerPawnComponent = FindComponentByClass<UNotoPlayerPawnComponent>())
	{
		PlayerPawnComponent->InitializePlayerInput(PlayerInputComponent);
	}
}
