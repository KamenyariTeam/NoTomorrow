"""Read-only validation of canonical promoted-animation ownership paths."""

import unreal


STAGING_ROOT = "/Game/Animation/Retargeted/Lyra"
CANONICAL_ROOTS = (
    "/Game/Animation/AimOffsets/Pistol",
    "/Game/Animation/AimOffsets/Rifle",
    "/Game/Animation/Reactions",
    "/Game/Animation/Weapons/Pistol",
    "/Game/Animation/Weapons/Rifle",
    "/Game/Animation/Weapons/Shotgun",
    "/Game/Animation/Weapons/Throwable",
)
TARGET_SKELETON = "/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin"


def main():
    staging_assets = unreal.EditorAssetLibrary.list_assets(STAGING_ROOT, recursive=True, include_folder=False)
    if staging_assets:
        raise RuntimeError("Promoted assets remain in staging: {}".format(staging_assets))
    target_skeleton = unreal.load_asset(TARGET_SKELETON)
    count = 0
    for root in CANONICAL_ROOTS:
        for path in unreal.EditorAssetLibrary.list_assets(root, recursive=False, include_folder=False):
            asset = unreal.load_asset(path)
            count += 1
            if asset.get_class().get_name() in ("AnimSequence", "AimOffsetBlendSpace"):
                if asset.get_editor_property("skeleton") != target_skeleton:
                    raise RuntimeError("Wrong skeleton for {}".format(path))
            if asset.get_class().get_name() == "AimOffsetBlendSpace":
                options = unreal.AssetRegistryDependencyOptions(
                    include_hard_package_references=True,
                    include_soft_package_references=True,
                )
                dependencies = unreal.AssetRegistryHelpers.get_asset_registry().get_dependencies(
                    asset.get_outermost().get_name(), options)
                unexpected = [str(dep) for dep in dependencies if str(dep).startswith("/Game/Characters/Heroes")]
                if unexpected:
                    raise RuntimeError("Aim Offset source dependencies remain on {}: {}".format(path, unexpected))
    if count != 100:
        raise RuntimeError("Expected 100 promoted assets, found {}".format(count))
    unreal.log("Canonical promoted layout validated: {} assets".format(count))


if __name__ == "__main__":
    main()
