// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace NotoGameplayTags
{
	NOTOMORROWGAME_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// Declare all of the custom native tags
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);

	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
	NOTOMORROWGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);
}
