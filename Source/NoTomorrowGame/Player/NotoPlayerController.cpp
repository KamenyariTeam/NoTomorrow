// © 2025 Kamenyari. All rights reserved.


#include "NotoPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerController)

void ANotoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	// Cursor aiming uses the absolute cursor position. Game-and-UI mode keeps
	// normal gameplay input active while allowing Slate widgets to handle input.
	SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}
