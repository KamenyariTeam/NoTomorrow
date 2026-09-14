"""Read-only scoped redirector audit for assets changed by the UEFN cutover."""

import unreal


ROOTS = (
    "/Game/Animation",
    "/Game/Characters/UEFN_Mannequin",
    "/Game/Characters/Heroes",
    "/Game/Characters/Mannequins",
)


def main():
    redirectors = []
    for root in ROOTS:
        for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
            asset = unreal.load_asset(path)
            if asset and asset.get_class().get_name() == "ObjectRedirector":
                redirectors.append(path)
    if redirectors:
        raise RuntimeError("Redirectors remain in changed content: {}".format(redirectors))
    unreal.log("No redirectors in changed content roots")


if __name__ == "__main__":
    main()
