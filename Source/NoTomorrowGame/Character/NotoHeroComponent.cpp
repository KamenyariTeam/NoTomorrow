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

	// De-project the cursor to a world-space ray.
	if (PC->DeprojectMousePositionToWorld(MouseWorldPosition, MouseWorldDirection))
	{
		// Intersect the ray with the horizontal plane at the pawn’s Z height.
		const float PawnZ = Pawn->GetActorLocation().Z;

		// Guard against rays parallel to the plane.
		if (!FMath::IsNearlyZero(MouseWorldDirection.Z))
		{
			const float Distance = (PawnZ - MouseWorldPosition.Z) / MouseWorldDirection.Z;

			// Only use intersections that lie in front of the camera.
			if (Distance > 0.0f)
			{
				const FVector HitPosition = MouseWorldPosition + MouseWorldDirection * Distance;

				// Calculate the 2D direction from pawn to intersection point.
				FVector DirectionToMouse = HitPosition - Pawn->GetActorLocation();
				DirectionToMouse.Z = 0.0f;
				DirectionToMouse.Normalize();

				if (!DirectionToMouse.IsNearlyZero())
				{
					const FRotator NewRotation = DirectionToMouse.Rotation();

					// Apply only the yaw so the pawn faces the cursor.
					Pawn->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
				}
			}
		}
	}
}
