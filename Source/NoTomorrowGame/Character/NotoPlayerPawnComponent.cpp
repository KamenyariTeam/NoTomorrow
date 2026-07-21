// © 2025 Kamenyari. All rights reserved.

#include "NotoPlayerPawnComponent.h"

#include "Perception/AISense_Hearing.h"
#include "Development/NotoGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Input/NotoInputComponent.h"
#include "Player/NotoPlayerController.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerPawnComponent)

UNotoPlayerPawnComponent::UNotoPlayerPawnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	MovementStates = {
		{NotoGameplayTags::MovementState_Run, 600.0f, 300.0f, NotoGameplayTags::NoiseTag_Movement},
		{NotoGameplayTags::MovementState_Sneak, 300.0f, 0.0f, NotoGameplayTags::NoiseTag_Movement}
	};
	MovementState = NotoGameplayTags::MovementState_Run;
}

void UNotoPlayerPawnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandlePawnControllerChanged);
	}

	RefreshAimTickEnabled();
	RefreshMovementNoiseTimer();
	ApplyMovementState();
}

void UNotoPlayerPawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandlePawnControllerChanged);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MovementNoiseTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UNotoPlayerPawnComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	const APawn* Pawn = GetPawn<APawn>();
	UNotoInputComponent* NotoInputComponent = Cast<UNotoInputComponent>(PlayerInputComponent);
	if (!ensure(Pawn && Pawn->IsLocallyControlled())
		|| !ensureMsgf(NotoInputComponent, TEXT("Native input bindings require UNotoInputComponent or a subclass."))
		|| !ensure(DefaultInputConfig))
	{
		return;
	}

	NotoInputComponent->ClearBindingsForObject(this);
	NotoInputComponent->BindNativeAction(
		DefaultInputConfig,
		NotoGameplayTags::InputTag_Move,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Move);
	NotoInputComponent->BindNativeAction(
		DefaultInputConfig,
		NotoGameplayTags::InputTag_Aim,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Aim);
	NotoInputComponent->BindNativeAction(
		DefaultInputConfig,
		NotoGameplayTags::InputTag_Sneak,
		ETriggerEvent::Started,
		this,
		&ThisClass::Input_ToggleSneak);

	RefreshAimTickEnabled();
}

void UNotoPlayerPawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAimFromMouseCursor();
}

void UNotoPlayerPawnComponent::HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	RefreshAimTickEnabled();
	RefreshMovementNoiseTimer();
}

void UNotoPlayerPawnComponent::RefreshAimTickEnabled()
{
	const APawn* Pawn = GetPawn<APawn>();
	SetComponentTickEnabled(Pawn && Pawn->IsLocallyControlled());
}

void UNotoPlayerPawnComponent::RefreshMovementNoiseTimer()
{
	const APawn* Pawn = GetPawn<APawn>();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MovementNoiseTimer);
		if (Pawn && Pawn->IsLocallyControlled() && MovementNoiseInterval > 0.0f)
		{
			World->GetTimerManager().SetTimer(MovementNoiseTimer, this, &ThisClass::ReportMovementNoise, MovementNoiseInterval, true);
		}
	}
}

void UNotoPlayerPawnComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	const AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (!Controller)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (!FMath::IsNearlyZero(Value.X))
	{
		Pawn->AddMovementInput(MovementRotation.RotateVector(FVector::RightVector), Value.X);
	}

	if (!FMath::IsNearlyZero(Value.Y))
	{
		Pawn->AddMovementInput(MovementRotation.RotateVector(FVector::ForwardVector), Value.Y);
	}
}

void UNotoPlayerPawnComponent::Input_Aim(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	ANotoPlayerController* PlayerController = Pawn ? Cast<ANotoPlayerController>(Pawn->GetController()) : nullptr;
	if (!PlayerController || !PlayerController->IsUsingGamepad())
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator AimRotation(0.0f, PlayerController->GetControlRotation().Yaw, 0.0f);
	const FVector AimDirection = AimRotation.RotateVector(FVector(Value.Y, Value.X, 0.0f)).GetSafeNormal2D();
	Pawn->SetActorRotation(AimDirection.Rotation());

	FVector2D CrosshairPosition;
	if (PlayerController->ProjectWorldLocationToScreen(Pawn->GetActorLocation() + AimDirection * GamepadAimRadius, CrosshairPosition, true))
	{
		PlayerController->SetGameplayReticlePosition(CrosshairPosition);
	}
}

void UNotoPlayerPawnComponent::Input_ToggleSneak()
{
	SetMovementState(MovementState == NotoGameplayTags::MovementState_Sneak ? NotoGameplayTags::MovementState_Run : NotoGameplayTags::MovementState_Sneak);
}

void UNotoPlayerPawnComponent::SetMovementState(FGameplayTag NewStateTag)
{
	if (MovementState == NewStateTag || !FindMovementStateConfig(NewStateTag))
	{
		return;
	}

	MovementState = NewStateTag;
	ApplyMovementState();
}

void UNotoPlayerPawnComponent::UpdateAimFromMouseCursor()
{
	APawn* Pawn = GetPawn<APawn>();
	ANotoPlayerController* PlayerController = Pawn ? Cast<ANotoPlayerController>(Pawn->GetController()) : nullptr;
	if (!PlayerController || PlayerController->IsUsingGamepad())
	{
		return;
	}

	FVector AimDirection;
	if (GetMouseAimDirection(*PlayerController, *Pawn, AimDirection))
	{
		Pawn->SetActorRotation(AimDirection.Rotation());
	}

	float MouseX;
	float MouseY;
	if (PlayerController->GetMousePosition(MouseX, MouseY))
	{
		PlayerController->SetGameplayReticlePosition(FVector2D(MouseX, MouseY));
	}
}

bool UNotoPlayerPawnComponent::GetMouseAimDirection(const APlayerController& PlayerController, const APawn& Pawn, FVector& OutAimDirection) const
{
	FVector RayOrigin;
	FVector RayDirection;
	if (!PlayerController.DeprojectMousePositionToWorld(RayOrigin, RayDirection))
	{
		return false;
	}

	const FVector PawnLocation = Pawn.GetActorLocation();
	const double IntersectionDistance = FMath::RayPlaneIntersectionParam(RayOrigin, RayDirection, FPlane(PawnLocation, FVector::UpVector));
	if (IntersectionDistance <= 0.0)
	{
		return false;
	}

	OutAimDirection = (RayOrigin + RayDirection * IntersectionDistance - PawnLocation).GetSafeNormal2D();
	return !OutAimDirection.IsNearlyZero();
}

const FNotoMovementStateConfig* UNotoPlayerPawnComponent::FindMovementStateConfig(FGameplayTag StateTag) const
{
	return MovementStates.FindByPredicate([StateTag](const FNotoMovementStateConfig& Config)
	{
		return Config.StateTag == StateTag;
	});
}

void UNotoPlayerPawnComponent::ApplyMovementState()
{
	const FNotoMovementStateConfig* Config = FindMovementStateConfig(MovementState);
	ACharacter* Character = GetPawn<ACharacter>();
	if (Config && Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Config->MaxWalkSpeed;
	}
}

void UNotoPlayerPawnComponent::ReportMovementNoise()
{
	APawn* Pawn = GetPawn<APawn>();
	const FNotoMovementStateConfig* Config = FindMovementStateConfig(MovementState);
	if (!Pawn || !Config || Config->NoiseRange <= 0.0f || Pawn->GetVelocity().IsNearlyZero())
	{
		return;
	}
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), Pawn->GetActorLocation(), 1.0f, Pawn, Config->NoiseRange, Config->NoiseTag.GetTagName());
}
