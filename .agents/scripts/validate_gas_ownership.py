import unreal

game_mode_blueprint = unreal.EditorAssetLibrary.load_asset("/Game/BP_NotoGameMode")
game_mode_cdo = unreal.get_default_object(game_mode_blueprint.generated_class())
player_state_class = game_mode_cdo.get_editor_property("player_state_class")
assert player_state_class.get_path_name() == "/Script/NoTomorrowGame.NotoPlayerState"

player_state_cdo = unreal.get_default_object(player_state_class)
player_state_components = player_state_cdo.get_components_by_class(unreal.AbilitySystemComponent)
assert len(player_state_components) == 1
assert player_state_components[0].get_editor_property("replicates")

character_blueprint = unreal.EditorAssetLibrary.load_asset("/Game/Characters/Heroes/BP_Character_Default")
character_cdo = unreal.get_default_object(character_blueprint.generated_class())
assert len(character_cdo.get_components_by_class(unreal.AbilitySystemComponent)) == 0

unreal.log("GAS ownership validated: one replicated PlayerState ASC, no Character ASC")
