// © 2025 Kamenyari. All rights reserved.


#include "NotoPlayerController.h"

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