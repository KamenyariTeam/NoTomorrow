import unreal


def main():
    blueprint = unreal.load_asset("/Game/Blueprints/SandboxCharacter_CMC_ABP")
    anim_graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, "AnimGraph")
    if not anim_graph:
        raise RuntimeError("SandboxCharacter_CMC_ABP has no AnimGraph")

    slots = anim_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_Slot)
    if len(slots) != 1:
        raise RuntimeError("Expected exactly one active locomotion slot node")
    for slot in slots:
        slot_node = slot.get_editor_property("node")
        slot_name = str(slot_node.get_editor_property("slot_name"))
        if slot_name != "WeaponUpperBody":
            raise RuntimeError("Active locomotion slot is {} instead of WeaponUpperBody".format(slot_name))
        unreal.log("AnimGraph slot {} slot_name={}".format(slot.get_name(), slot_name))

    if not unreal.BlueprintEditorLibrary.compile_blueprint(blueprint):
        raise RuntimeError("SandboxCharacter_CMC_ABP did not compile")
    unreal.EditorAssetLibrary.save_asset("/Game/Blueprints/SandboxCharacter_CMC_ABP", only_if_is_dirty=False)

    character = unreal.load_asset("/Game/Characters/Heroes/BP_Character_Default")
    subobjects = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subobjects.k2_gather_subobject_data_for_blueprint(character)
    traversal_handle = None
    for handle in handles:
        data = subobjects.k2_find_subobject_data_from_handle(handle)
        name = unreal.SubobjectDataBlueprintFunctionLibrary.get_variable_name(data)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_object_for_blueprint(data, character)
        unreal.log("Character subobject {} class={}".format(
            name, obj.get_class().get_name() if obj else "None"))
        if str(name) == "Traversal Logic Component":
            traversal_handle = handle

    if traversal_handle is not None:
        raise RuntimeError("Traversal Logic Component is still attached to BP_Character_Default")
    if not unreal.BlueprintEditorLibrary.compile_blueprint(character):
        raise RuntimeError("BP_Character_Default did not compile")
    unreal.EditorAssetLibrary.save_asset("/Game/Characters/Heroes/BP_Character_Default", only_if_is_dirty=False)


if __name__ == "__main__":
    main()
