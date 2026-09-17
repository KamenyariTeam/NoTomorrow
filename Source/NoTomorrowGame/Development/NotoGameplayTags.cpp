// © 2025 Kamenyari. All rights reserved.

#include "NotoGameplayTags.h"

namespace NotoGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Aim, "InputTag.Aim", "Aim input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "Jump input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Toggle crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sprint, "InputTag.Sprint", "Hold sprint input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Walk, "InputTag.Walk", "Toggle walk input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Interact, "InputTag.Interact", "Interact with the selected world object.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_InteractModified, "InputTag.InteractModified", "Interact using the configured interaction modifier.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Drop, "InputTag.Drop", "Drop the active equipped item.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Fire, "InputTag.Fire", "Fire the active weapon.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Reload, "InputTag.Reload", "Reload the active weapon.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NoiseTag_Movement, "Noise.Movement", "Noise emitted by moving pawns.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NoiseTag_Gunfire, "Noise.Gunfire", "Noise emitted by firearms.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ammo_Light, "Ammo.Light", "Light ammunition family.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ammo_Heavy, "Ammo.Heavy", "Heavy ammunition family.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ammo_Shell, "Ammo.Shell", "Shell ammunition family.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Weapon_Rifle, "Item.Weapon.Rifle", "Rifle weapon family.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemAction_Equip, "Item.Action.Equip", "An item became active in the character's hands.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemAction_Fire, "Item.Action.Fire", "The active firearm fired successfully.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemAction_DryFire, "Item.Action.DryFire", "The active firearm attempted to fire without enough loaded ammunition.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemAction_Reload, "Item.Action.Reload", "The active firearm reloaded successfully.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage", "Set-by-caller damage magnitude.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "Actor health has reached zero.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Ragdoll, "State.Ragdoll", "Actor is simulated by the ragdoll component.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Traversing, "State.Traversing", "Actor is performing a traversal action.");
}
