// © 2025 Kamenyari. All rights reserved.

#include "NotoHeroComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Development/NotoGameplayTags.h"
#include "Development/NotoLogChannels.h"
#include "Input/NotoInputComponent.h"
#include "Misc/UObjectToken.h"

namespace NotoHero
{
	// Rates used for look input adjustments.
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
}

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

	// TODO: Disable this when we are not using the mouse.
	Input_LookMouse();
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
		// Unfortunately, Mouse 2D input is not working correctly while cursor is showing.
		// NotoIC->BindNativeAction(DefaultInputConfig, NotoGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
		NotoIC->BindNativeAction(DefaultInputConfig, NotoGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
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

void UNotoHeroComponent::Input_LookMouse(/*const FInputActionValue& InputActionValue*/)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		return;
	}

	FVector MouseWorldPosition;
	FVector MouseWorldDirection;

	// Get the mouse position in the world space
	if (PC->DeprojectMousePositionToWorld(MouseWorldPosition, MouseWorldDirection))
	{
		// Get the pawn's current location
		FVector PawnLocation = Pawn->GetActorLocation();

		// Calculate the direction from the pawn to the mouse position
		FVector DirectionToMouse = MouseWorldPosition - PawnLocation;
		DirectionToMouse.Z = 0.0f; // Keep it 2D (top-down), ignore Z axis

		// Normalize the direction to get the direction vector
		DirectionToMouse.Normalize();

		// Get the rotation to face the mouse direction
		FRotator NewRotation = DirectionToMouse.Rotation();

		// Apply the new yaw rotation to the pawn
		Pawn->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f)); // Set only Yaw for top-down
	}
}

void UNotoHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !GetWorld())
	{
		return;
	}

	const FVector2D StickInput = InputActionValue.Get<FVector2D>();
    
	//const FVector2D ClampedInput = StickInput.GetClampedToMaxSize(1.0f);
	// const FVector WorldSpaceDirection = FVector(ClampedInput.X, ClampedInput.Y, 0.f);
	const FVector WorldSpaceDirection = FVector(StickInput.X, StickInput.Y, 0.f);

	// Convert to rotation (Hotline Miami-style direct facing)
	const FRotator TargetRotation = WorldSpaceDirection.Rotation();
    
	// Immediate rotation (no smoothing)
	Pawn->SetActorRotation(TargetRotation);

	/* For smoothed rotation (optional):
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	const FRotator NewRotation = FMath::RInterpTo(
		Pawn->GetActorRotation(),
		TargetRotation,
		DeltaTime,
		NotoHero::LookTurnRate // Define this as your rotation speed (e.g., 15.0f)
	);
	Pawn->SetActorRotation(NewRotation);
	*/
}
