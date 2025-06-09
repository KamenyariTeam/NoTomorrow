// © 2025 Kamenyari. All rights reserved.

#include "NotoCharacter.h"
#include "AbilitySystem/NotoAbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "NotoCharacterMovementComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "Player/NotoPlayerController.h"
#include "Player/NotoPlayerState.h"

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

	NotoAbilitySystemComp = CreateDefaultSubobject<UNotoAbilitySystemComponent>(TEXT("AbilitySystem"));
	
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

ANotoPlayerState* ANotoCharacter::GetNotoPlayerState() const
{
	return CastChecked<ANotoPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

void ANotoCharacter::BeginPlay()
{
	Super::BeginPlay();

	NotoAbilitySystemComp->InitAbilityActorInfo(this, this);
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

void ANotoCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	HandleControllerChanged();
}

void ANotoCharacter::UnPossessed()
{
	Super::UnPossessed();
	HandleControllerChanged();
}

void ANotoCharacter::HandleControllerChanged()
{
	if (NotoAbilitySystemComp && (NotoAbilitySystemComp->GetAvatarActor() == this))
	{
		ensure(NotoAbilitySystemComp->AbilityActorInfo->OwnerActor == NotoAbilitySystemComp->GetOwnerActor());
		if (NotoAbilitySystemComp->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			NotoAbilitySystemComp->RefreshAbilityActorInfo();
		}
	}
}

void ANotoCharacter::UninitializeAbilitySystem()
{
	// [TODO][m.khavro] implement logic for unitializing ACS
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

UAbilitySystemComponent* ANotoCharacter::GetAbilitySystemComponent() const
{
	return GetNotoAbilitySystemComponent();
}
