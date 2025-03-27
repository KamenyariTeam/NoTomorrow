// © 2025 Kamenyari. All rights reserved.


#include "NotoPlayerController.h"

#include "UI/NotoHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NotoPlayerController)

namespace Noto
{
	namespace Input
	{
		static int32 ShouldAlwaysPlayForceFeedback = 0;
		static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("NotoPC.ShouldAlwaysPlayForceFeedback"),
			ShouldAlwaysPlayForceFeedback,
			TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
	}
}

ANotoPlayerController::ANotoPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ANotoPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// Show the cursor and lock it to the game viewport
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false); // Keep cursor visible
	SetInputMode(InputMode);
}

ANotoHUD* ANotoPlayerController::GetNotoHUD() const
{
	return CastChecked<ANotoHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

void ANotoPlayerController::AddCheats(bool bForce)
{
// #if USING_CHEAT_MANAGER
// 	Super::AddCheats(true);
// #else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
// #endif // #else //#if USING_CHEAT_MANAGER
}
