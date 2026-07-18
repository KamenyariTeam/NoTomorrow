// © 2025 Kamenyari. All rights reserved.

#include "NotoPlayerPawnComponent.h"

#include "Development/NotoGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "Input/NotoInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerPawnComponent)

UNotoPlayerPawnComponent::UNotoPlayerPawnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNotoPlayerPawnComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandlePawnControllerChanged);
	}

	RefreshTickEnabled();
}

void UNotoPlayerPawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandlePawnControllerChanged);
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

	RefreshTickEnabled();
}

void UNotoPlayerPawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAimFromMouseCursor();
}

void UNotoPlayerPawnComponent::HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	RefreshTickEnabled();
}

void UNotoPlayerPawnComponent::RefreshTickEnabled()
{
	const APawn* Pawn = GetPawn<APawn>();
	SetComponentTickEnabled(bAimWithMouseCursor && Pawn && Pawn->IsLocallyControlled());
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

void UNotoPlayerPawnComponent::UpdateAimFromMouseCursor()
{
	APawn* Pawn = GetPawn<APawn>();
	const APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PlayerController || !PlayerController->ShouldShowMouseCursor())
	{
		return;
	}

	FVector AimDirection;
	if (GetMouseAimDirection(*PlayerController, *Pawn, AimDirection))
	{
		Pawn->SetActorRotation(AimDirection.Rotation());
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
