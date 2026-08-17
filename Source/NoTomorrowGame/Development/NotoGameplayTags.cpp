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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Fire, "InputTag.Fire", "Fire the active weapon.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Reload, "InputTag.Reload", "Reload the active weapon.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MovementState_Run, "MovementState.Run", "Default fast movement.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MovementState_Sneak, "MovementState.Sneak", "Slow, silent movement.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NoiseTag_Movement, "Noise.Movement", "Noise emitted by moving pawns.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NoiseTag_Gunfire, "Noise.Gunfire", "Noise emitted by firearms.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ammo_Magazine_Light, "Ammo.Magazine.Light", "Standard light detachable magazine family.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ammo_Magazine_Heavy, "Ammo.Magazine.Heavy", "Standard heavy detachable magazine family.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ammo_Shell, "Ammo.Shell", "Loose shell ammunition family.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage", "Set-by-caller damage magnitude.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "Actor health has reached zero.");
}
