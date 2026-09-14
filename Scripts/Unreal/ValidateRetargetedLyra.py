"""Read-only validation of representative promoted Lyra categories."""

import unreal


SOURCE_TARGET_PAIRS = (
    ("Pistol fire", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_Pistol_Fire", "/Game/Animation/Weapons/Pistol/MM_Pistol_Fire"),
    ("Rifle reload", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_Rifle_Reload", "/Game/Animation/Weapons/Rifle/MM_Rifle_Reload"),
    ("Shotgun fire", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_Shotgun_Fire", "/Game/Animation/Weapons/Shotgun/MM_Shotgun_Fire"),
    ("Grenade", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_Rifle_GrenadeToss", "/Game/Animation/Weapons/Throwable/MM_Rifle_GrenadeToss"),
    ("Melee", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_Pistol_Melee", "/Game/Animation/Weapons/Pistol/MM_Pistol_Melee"),
    ("Hit reaction", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_HitReact_Front_Lgt_01", "/Game/Animation/Reactions/MM_HitReact_Front_Lgt_01"),
    ("Death", "/Game/Characters/Heroes/Mannequin/Animations/Actions/MM_Death_Back_01", "/Game/Animation/Reactions/MM_Death_Back_01"),
    ("Pistol aim offset", "/Game/Characters/Heroes/Mannequin/Animations/AimOffsets/AO_MM_Pistol_Idle_ADS", "/Game/Animation/AimOffsets/Pistol/AO_MM_Pistol_Idle_ADS"),
    ("Rifle aim offset", "/Game/Characters/Heroes/Mannequin/Animations/AimOffsets/AO_MM_Rifle_Idle_ADS", "/Game/Animation/AimOffsets/Rifle/AO_MM_Rifle_Idle_ADS"),
)
TARGET_SKELETON = "/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin"


def asset(path):
    result = unreal.load_asset(path)
    if not result:
        raise RuntimeError("Missing {}".format(path))
    return result


def property_value(item, name):
    try:
        return item.get_editor_property(name)
    except Exception:
        return None


def count_value(item, name):
    value = property_value(item, name)
    try:
        return len(value)
    except TypeError:
        return None


def validate_anim(label, source, target, failures):
    for name in ("additive_anim_type", "ref_pose_type", "ref_frame_index"):
        if property_value(source, name) != property_value(target, name):
            failures.append("{}: {} changed".format(label, name))
    if abs(source.get_play_length() - target.get_play_length()) > 0.001:
        failures.append("{}: play length changed".format(label))

    options = unreal.AnimPoseEvaluationOptions()
    pose = target.get_anim_pose_at_frame(0, options)
    attach = pose.get_bone_pose("attach", unreal.AnimPoseSpaces.LOCAL)
    attach_ref = pose.get_ref_bone_pose("attach", unreal.AnimPoseSpaces.LOCAL)
    if (attach.translation - attach_ref.translation).length() > 0.1:
        failures.append("{}: attach transform is animated".format(label))
    abnormal_scale_bones = []
    for bone in pose.get_bone_names():
        scale = pose.get_bone_pose(bone, unreal.AnimPoseSpaces.LOCAL).scale3d
        if abs(scale.x - 1.0) > 0.001 or abs(scale.y - 1.0) > 0.001 or abs(scale.z - 1.0) > 0.001:
            abnormal_scale_bones.append(str(bone))
    if abnormal_scale_bones:
        failures.append("{}: abnormal scale on {}".format(label, abnormal_scale_bones))

    source_curves = sorted(str(name) for name in source.get_anim_pose_at_frame(0, options).get_curve_names())
    target_curves = sorted(str(name) for name in pose.get_curve_names())
    if source_curves != target_curves:
        failures.append("{}: curve names changed".format(label))
    unreal.log("VALIDATE {} additive={} ref_pose={} length={} notifies={} curves={}".format(
        label, property_value(target, "additive_anim_type"), property_value(target, "ref_pose_type"),
        target.get_play_length(), count_value(target, "notifies"), target_curves))


def main():
    target_skeleton = asset(TARGET_SKELETON)
    failures = []
    for label, source_path, target_path in SOURCE_TARGET_PAIRS:
        source = asset(source_path)
        target = asset(target_path)
        if source.get_class() != target.get_class():
            failures.append("{}: asset class changed".format(label))
        if property_value(target, "skeleton") != target_skeleton:
            failures.append("{}: target skeleton mismatch".format(label))
        if target.get_class().get_name() == "AnimSequence":
            validate_anim(label, source, target, failures)
        else:
            unreal.log("VALIDATE {} class={} skeleton={}".format(
                label, target.get_class().get_name(), property_value(target, "skeleton")))
    if failures:
        raise RuntimeError("Retarget validation failed: {}".format("; ".join(failures)))
    unreal.log("Retarget representative validation passed for {} categories".format(len(SOURCE_TARGET_PAIRS)))


if __name__ == "__main__":
    main()
