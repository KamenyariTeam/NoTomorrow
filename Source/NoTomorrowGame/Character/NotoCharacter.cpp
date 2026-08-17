// © 2025 Kamenyari. All rights reserved.

#include "NotoCharacter.h"

#include "AbilitySystemComponent.h"
#include "Combat/NotoFirearmComponent.h"
#include "Combat/NotoHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "NotoPlayerPawnComponent.h"
#include "Interaction/NotoInteractionComponent.h"
#include "Player/NotoPlayerState.h"

ANotoCharacter::ANotoCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	check(Capsule);
	Capsule->InitCapsuleSize(40.0f, 90.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		MeshComponent->SetCollisionProfileName(TEXT("PawnMesh"));
	}

	GameplayCameraComponent = CreateDefaultSubobject<UGameplayCameraComponent>(TEXT("CameraComponent"));
	GameplayCameraComponent->SetupAttachment(RootComponent);
	GameplayCameraComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	InteractionComponent = CreateDefaultSubobject<UNotoInteractionComponent>(TEXT("InteractionComponent"));
	HealthComponent = CreateDefaultSubobject<UNotoHealthComponent>(TEXT("HealthComponent"));
	FirearmComponent = CreateDefaultSubobject<UNotoFirearmComponent>(TEXT("FirearmComponent"));
	PlayerPawnComponent = CreateDefaultSubobject<UNotoPlayerPawnComponent>(TEXT("PlayerPawnComponent"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

UAbilitySystemComponent* ANotoCharacter::GetAbilitySystemComponent() const
{
	const ANotoPlayerState* NotoPlayerState = GetPlayerState<ANotoPlayerState>();
	return NotoPlayerState ? NotoPlayerState->GetAbilitySystemComponent() : nullptr;
}

void ANotoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerPawnComponent)
	{
		PlayerPawnComponent->InitializePlayerInput(PlayerInputComponent);
	}
}

void ANotoCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void ANotoCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

void ANotoCharacter::UnPossessed()
{
	UninitializeAbilitySystem();
	Super::UnPossessed();
}

void ANotoCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void ANotoCharacter::InitializeAbilitySystem()
{
	ANotoPlayerState* NotoPlayerState = GetPlayerState<ANotoPlayerState>();
	if (!NotoPlayerState)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = NotoPlayerState->GetAbilitySystemComponent();
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(NotoPlayerState, this);
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void ANotoCharacter::UninitializeAbilitySystem()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
		AbilitySystemComponent && AbilitySystemComponent->GetAvatarActor() == this)
	{
		HealthComponent->UninitializeFromAbilitySystem();
		AbilitySystemComponent->ClearActorInfo();
	}
}
