// © 2025 Kamenyari. All rights reserved.

#include "NotoCharacter.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "NotoCharacterMovementComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/NotoPlayerController.h"

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
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		MeshComp->SetCollisionProfileName(TEXT("PawnMesh"));
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TopDownSpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1000.0f;
	SpringArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); // Look downward
	SpringArm->bDoCollisionTest = false; // Disable camera collision

	// Disable inheriting rotation from the pawn
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

ANotoPlayerController* ANotoCharacter::GetNotoPlayerController() const
{
	return Cast<ANotoPlayerController>(Controller);
}

void ANotoCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Additional initialization can occur here.
}

void ANotoCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// Clean-up code can go here.
}

void ANotoCharacter::Reset()
{
	// Disable movement and collision, then reset the character.
	DisableMovementAndCollision();
}

void ANotoCharacter::DisableMovementAndCollision()
{
	// Ignore further movement input.
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	// Disable collision on the capsule.
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Stop movement immediately and disable movement.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
}
