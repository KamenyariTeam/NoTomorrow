// © 2025 Kamenyari. All rights reserved.

#include "NotoHeroComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Development/NotoGameplayTags.h"
#include "Development/NotoLogChannels.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Input/NotoInputComponent.h"
#include "Misc/UObjectToken.h"

const FName UNotoHeroComponent::NAME_BindInputsNow("BindInputsNow");

UNotoHeroComponent::UNotoHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bReadyToBindInputs(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNotoHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogNoto, Error, TEXT("[UNotoHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("NotoHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("NotoHeroComponent");
			
			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
				
			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
}

void UNotoHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandlePawnControllerChanged);
		Pawn->ReceiveRestartedDelegate.AddUniqueDynamic(this, &ThisClass::HandlePawnRestarted);
	}

	TryInitializePlayerInput();
}

void UNotoHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandlePawnControllerChanged);
		Pawn->ReceiveRestartedDelegate.RemoveDynamic(this, &ThisClass::HandlePawnRestarted);
	}

	ResetPlayerInputBindings();

	Super::EndPlay(EndPlayReason);
}

void UNotoHeroComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TryInitializePlayerInput();
	UpdateAimFromMouseCursor();
}

void UNotoHeroComponent::TryInitializePlayerInput()
{
	APawn* Pawn = GetPawn<APawn>();
	APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	UInputComponent* PlayerInputComponent = Pawn ? Pawn->InputComponent : nullptr;

	if (bReadyToBindInputs && Pawn && Pawn->IsLocallyControlled() && BoundPlayerController.Get() == PlayerController && BoundInputComponent.Get() == PlayerInputComponent)
	{
		return;
	}

	if (bReadyToBindInputs)
	{
		ResetPlayerInputBindings();
	}

	if (!Pawn || !Pawn->IsLocallyControlled() || !PlayerController || !PlayerInputComponent)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer || !LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		return;
	}

	UNotoInputComponent* NotoIC = Cast<UNotoInputComponent>(PlayerInputComponent);
	if (!ensureMsgf(NotoIC, TEXT("Unexpected Input Component class! Native input bindings require UNotoInputComponent or a subclass.")) || !ensure(DefaultInputConfig))
	{
		return;
	}

	if (!NotoIC->BindNativeAction(DefaultInputConfig, NotoGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ true))
	{
		return;
	}

	bReadyToBindInputs = true;
	BoundPlayerController = PlayerController;
	BoundInputComponent = PlayerInputComponent;

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(PlayerController, NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_BindInputsNow);
}

void UNotoHeroComponent::ResetPlayerInputBindings()
{
	if (UNotoInputComponent* NotoInputComponent = Cast<UNotoInputComponent>(BoundInputComponent.Get()))
	{
		NotoInputComponent->ClearBindingsForObject(this);
	}

	bReadyToBindInputs = false;
	BoundPlayerController.Reset();
	BoundInputComponent.Reset();
}

void UNotoHeroComponent::HandlePawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	ResetPlayerInputBindings();
	TryInitializePlayerInput();
}

void UNotoHeroComponent::HandlePawnRestarted(APawn* Pawn)
{
	TryInitializePlayerInput();
}

bool UNotoHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

//
// Input Callbacks
//

void UNotoHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (!Controller)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	// Apply movement on the X axis (right/left).
	if (!FMath::IsNearlyZero(Value.X))
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		Pawn->AddMovementInput(MovementDirection, Value.X);
	}
	// Apply movement on the Y axis (forward/back).
	if (!FMath::IsNearlyZero(Value.Y))
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		Pawn->AddMovementInput(MovementDirection, Value.Y);
	}
}

void UNotoHeroComponent::UpdateAimFromMouseCursor()
{
	if (!bAimWithMouseCursor)
	{
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!PlayerController || !PlayerController->ShouldShowMouseCursor())
	{
		return;
	}

	FVector AimDirection;
	if (GetMouseAimDirection(*PlayerController, *Pawn, AimDirection))
	{
		// Control rotation remains the movement reference in Input_Move, so cursor
		// aiming rotates the pawn without changing the player's movement axes.
		Pawn->SetActorRotation(AimDirection.Rotation());
	}
}

bool UNotoHeroComponent::GetMouseAimDirection(const APlayerController& PlayerController, const APawn& Pawn, FVector& OutAimDirection) const
{
	FVector RayOrigin;
	FVector RayDirection;
	if (!PlayerController.DeprojectMousePositionToWorld(RayOrigin, RayDirection))
	{
		return false;
	}

	const FVector PawnLocation = Pawn.GetActorLocation();
	const FPlane AimPlane(PawnLocation, FVector::UpVector);
	const double IntersectionDistance = FMath::RayPlaneIntersectionParam(RayOrigin, RayDirection, AimPlane);
	if (IntersectionDistance <= 0.0)
	{
		return false;
	}

	const FVector MouseWorldPosition = RayOrigin + RayDirection * IntersectionDistance;
	OutAimDirection = (MouseWorldPosition - PawnLocation).GetSafeNormal2D();
	return !OutAimDirection.IsNearlyZero();
}
