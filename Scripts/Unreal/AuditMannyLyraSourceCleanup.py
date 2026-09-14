"""Read-only source-content audit for the final Manny/Lyra cleanup."""

import unreal


SOURCE_ROOTS = (
    "/Game/Characters/Heroes/Mannequin",
    "/Game/Characters/Mannequins/Anims",
    "/Game/Characters/Mannequins/Meshes",
    "/Game/Characters/Mannequins/Rigs",
)


def main():
    totals = {}
    for root in SOURCE_ROOTS:
        assets = unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)
        classes = {}
        external_referenced = []
        for path in assets:
            asset = unreal.load_asset(path)
            if not asset:
                continue
            class_name = asset.get_class().get_name()
            classes[class_name] = classes.get(class_name, 0) + 1
            refs = [str(ref) for ref in unreal.EditorAssetLibrary.find_package_referencers_for_asset(path, False)
                    if not str(ref).startswith(root)]
            if refs:
                external_referenced.append((path, refs))
        totals[root] = len(assets)
        unreal.log("SOURCE_AUDIT root={} total={} classes={} external_referenced={}".format(
            root, len(assets), classes, external_referenced))
    unreal.log("SOURCE_AUDIT totals={}".format(totals))


if __name__ == "__main__":
    main()
