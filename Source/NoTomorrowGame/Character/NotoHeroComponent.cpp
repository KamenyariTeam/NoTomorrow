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

	InitializePlayerInput();
}

void UNotoHeroComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAimFromMouseCursor();
}

void UNotoHeroComponent::InitializePlayerInput()
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	UInputComponent* PlayerInputComponent = Pawn->InputComponent;
	check(PlayerInputComponent);
	
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();
	
	// The Noto Input Component has some additional functions to map Gameplay Tags to an Input Action.
	// If you want this functionality but still want to change your input component class, make it a subclass
	// of the UNotoInputComponent or modify this component accordingly.
	UNotoInputComponent* NotoIC = Cast<UNotoInputComponent>(PlayerInputComponent);
	if (ensureMsgf(NotoIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UNotoInputComponent or a subclass of it.")))
	{
		NotoIC->BindNativeAction(DefaultInputConfig, NotoGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}
	
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
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
