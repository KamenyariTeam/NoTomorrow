// © 2025 Kamenyari. All rights reserved.

#include "NotoGameplayTags.h"

namespace NotoGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Aim, "InputTag.Aim", "Aim input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sneak, "InputTag.Sneak", "Toggle sneak movement.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact, "InputTag.Interact", "Interact with the selected world object.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_InteractModified, "InputTag.InteractModified", "Interact using the configured interaction modifier.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Drop, "InputTag.Drop", "Drop the active equipped item.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MovementState_Run, "MovementState.Run", "Default fast movement.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MovementState_Sneak, "MovementState.Sneak", "Slow, silent movement.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NoiseTag_Movement, "Noise.Movement", "Noise emitted by moving pawns.");
}
