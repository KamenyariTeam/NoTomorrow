// © 2025 Kamenyari. All rights reserved.

#include "NotoHeroComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "Input/NotoInputComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"

namespace NotoHero
{
	// Rates used for look input adjustments.
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

UNotoHeroComponent::UNotoHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bReadyToBindInputs(false)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UNotoHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UNotoHeroComponent::AddAdditionalInputConfig(const UNotoInputConfig* InInputConfig)
{
	if (!InInputConfig)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	if (UInputComponent* IC = Pawn->InputComponent)
	{
		UNotoInputComponent* NotoIC = Cast<UNotoInputComponent>(IC);
		if (NotoIC)
		{
			// Call a custom method to add extra input bindings from InInputConfig.
			// e.g., NotoIC->AddExtraInputMappings(InInputConfig);
		}
	}
}

void UNotoHeroComponent::RemoveAdditionalInputConfig(const UNotoInputConfig* InInputConfig)
{
	// Implement removal logic if needed.
}

void UNotoHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return;
	}

	// Get the Enhanced Input subsystem for this local player.
	UEnhancedInputLocalPlayerSubsystem* Subsystem = nullptr;
	if (PC->GetLocalPlayer())
	{
		Subsystem = PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	if (Subsystem)
	{
		// Clear existing mappings and add default mappings.
		Subsystem->ClearAllMappings();

		if (InputConfig)
		{
			// Add default input mapping contexts.
			for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
			{
				if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
				{
					if (Mapping.bRegisterWithSettings)
					{
						if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
						{
							Settings->RegisterInputMappingContext(IMC);
						}

						FModifyContextOptions Options = {};
						Options.bIgnoreAllPressedKeysUntilRelease = false;
						Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
					}
				}
			}

			// Ensure your custom input component (assumed to be UNotoInputComponent) is used.
			UNotoInputComponent* NotoIC = Cast<UNotoInputComponent>(PlayerInputComponent);
			if (NotoIC && InputConfig)
			{
				// Instead of hardcoding string literals, we now use gameplay tags.
				const FGameplayTag MoveTag = FGameplayTag::RequestGameplayTag(FName("InputTag_Move"));
				const FGameplayTag LookMouseTag = FGameplayTag::RequestGameplayTag(FName("InputTag_Look_Mouse"));
				const FGameplayTag LookStickTag = FGameplayTag::RequestGameplayTag(FName("InputTag_Look_Stick"));

				// Bind the actions using the unified input config.
				NotoIC->BindNativeAction(InputConfig, MoveTag, ETriggerEvent::Triggered, this, &UNotoHeroComponent::Input_Move, false);
				NotoIC->BindNativeAction(InputConfig, LookMouseTag, ETriggerEvent::Triggered, this, &UNotoHeroComponent::Input_LookMouse, false);
				NotoIC->BindNativeAction(InputConfig, LookStickTag, ETriggerEvent::Triggered, this, &UNotoHeroComponent::Input_LookStick, false);
			}
		}
	}

	bReadyToBindInputs = true;
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

void UNotoHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (!FMath::IsNearlyZero(Value.X))
	{
		Pawn->AddControllerYawInput(Value.X);
	}
	if (!FMath::IsNearlyZero(Value.Y))
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UNotoHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	if (!FMath::IsNearlyZero(Value.X))
	{
		Pawn->AddControllerYawInput(Value.X * NotoHero::LookYawRate * World->GetDeltaSeconds());
	}
	if (!FMath::IsNearlyZero(Value.Y))
	{
		Pawn->AddControllerPitchInput(Value.Y * NotoHero::LookPitchRate * World->GetDeltaSeconds());
	}
}
