"""Import original V78 FBX meshes and twelve clips per character in Unreal 5.6.

Run with UnrealEditor-Cmd -run=pythonscript -script=<this file>. Outside Unreal,
--preflight only checks source FBX files and never claims to create .uassets.
The importer preserves existing assets and validates every required package.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import sys
import traceback

ROLES = ("Hero", "Guard", "Raider")
CLIPS = ("Idle", "Walk", "Run", "Sprint", "Jump", "Dodge", "LightAttack",
         "HeavyAttack", "Hit", "Death", "Block", "Cast")
DESTINATION = "/Game/Askir/CharactersV78"


def fbx_sources(project_root):
    """One rig/mesh plus separate, unambiguous one-action FBX files per role."""
    source = Path(project_root) / "ArtSource/Animated_V78"
    return {role: {"mesh": source / ("Askir_" + role + ".fbx"),
                   "animations": {clip: source / ("Askir_" + role + "_" + clip + ".fbx")
                                  for clip in CLIPS}}
            for role in ROLES}


def inspect_fbx(path):
    path = Path(path)
    if not path.is_file() or path.stat().st_size < 128:
        raise RuntimeError("Required nonempty FBX source missing: " + str(path))
    with path.open("rb") as handle:
        header = handle.read(64)
    if not (header.startswith(b"Kaydara FBX Binary") or b"FBX" in header):
        raise RuntimeError("Source has no FBX header: " + str(path))
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return {"file": str(path), "bytes": path.stat().st_size, "sha256": digest.hexdigest()}


def source_preflight(project_root):
    sources = fbx_sources(project_root)
    checked = {}
    # Validate all inputs before any engine asset mutation.
    for role, inputs in sources.items():
        checked[role] = {"mesh": inspect_fbx(inputs["mesh"]),
                         "animations": {clip: inspect_fbx(path)
                                        for clip, path in inputs["animations"].items()}}
    return {"status": "Bestanden", "coverage": "FBX headers, sizes and source checksums only",
            "runtime_animation": "Nicht ausgeführt", "roles": checked}


def object_path(role, name):
    if role not in ROLES or not name or "/" in name or "." in name:
        raise ValueError("Invalid character asset name")
    package = DESTINATION + "/" + role + "/" + name
    return package + "." + name


def assert_package(project_root, asset_path):
    package = asset_path.split(".", 1)[0]
    if not package.startswith("/Game/Askir/CharactersV78/") or ".." in package:
        raise RuntimeError("Refusing non-character project package: " + package)
    path = Path(project_root) / "Content" / (package[len("/Game/"):] + ".uasset")
    if not path.is_file() or path.stat().st_size < 128:
        raise RuntimeError("Editor reported save but actual .uasset is missing/empty: " + str(path))
    return str(path)


def validate_sequence(sequence, skeleton, label):
    if sequence.get_editor_property("skeleton") != skeleton:
        raise RuntimeError("Animation skeleton mismatch: " + label)
    length = float(sequence.get_play_length())
    if not math.isfinite(length) or length <= 0:
        raise RuntimeError("Animation contains no finite positive playback duration: " + label)
    return length


def _import_task(unreal, file, role, name, skeleton=None):
    task = unreal.AssetImportTask()
    for key, value in {"filename": str(file), "destination_path": DESTINATION + "/" + role,
                       "destination_name": name, "automated": True,
                       "replace_existing": False, "save": True}.items():
        task.set_editor_property(key, value)
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_mesh", skeleton is None)
    options.set_editor_property("import_animations", skeleton is not None)
    options.set_editor_property("import_materials", skeleton is None)
    options.set_editor_property("import_textures", skeleton is None)
    if skeleton is None:
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
        data = options.get_editor_property("skeletal_mesh_import_data")
        data.set_editor_property("update_skeleton_reference_pose", False)
        data.set_editor_property("use_t0_as_ref_pose", False)
    else:
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
        # Each clip must use its mesh's actual skeleton, never an inferred similarly named rig.
        options.set_editor_property("skeleton", skeleton)
        data = options.get_editor_property("anim_sequence_import_data")
        data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
        data.set_editor_property("use_default_sample_rate", False)
        data.set_editor_property("custom_sample_rate", 30)
    data.set_editor_property("convert_scene", True)
    data.set_editor_property("convert_scene_unit", True)
    data.set_editor_property("force_front_x_axis", False)
    task.set_editor_property("options", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


def _load_or_import(unreal, project_root, file, role, name, expected_type, skeleton=None):
    assets = unreal.EditorAssetLibrary
    expected = object_path(role, name)
    existing = assets.does_asset_exist(expected)
    imported_paths = [] if existing else _import_task(unreal, file, role, name, skeleton)
    asset = unreal.load_asset(expected) if assets.does_asset_exist(expected) else None
    # FBX may append its exported action/armature name. Rename only the sole new asset
    # of the required type, never an existing or ambiguous imported sequence.
    if asset is None and not existing:
        candidates = [(path, unreal.load_asset(path)) for path in imported_paths]
        candidates = [(path, value) for path, value in candidates if isinstance(value, expected_type)]
        if len(candidates) != 1:
            raise RuntimeError("FBX did not produce exactly one required asset: " + expected)
        original, asset = candidates[0]
        if not assets.rename_asset(original, expected):
            raise RuntimeError("Cannot give imported asset its required runtime name: " + expected)
        asset = unreal.load_asset(expected)
    if not isinstance(asset, expected_type):
        raise RuntimeError("Existing/imported asset has unexpected type: " + expected)
    if not assets.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Required asset save failed: " + expected)
    assert_package(project_root, expected)
    return asset, expected


def import_assets(unreal, project_root):
    project_root = Path(project_root)
    version = unreal.SystemLibrary.get_engine_version()
    if not version.startswith("5.6."):
        raise RuntimeError("Unreal Engine 5.6 required, got " + version)
    source_validation = source_preflight(project_root)
    sources = fbx_sources(project_root)
    imported = {}
    for role, inputs in sources.items():
        mesh, mesh_path = _load_or_import(unreal, project_root, inputs["mesh"], role,
                                         "SK_Askir" + role, unreal.SkeletalMesh)
        skeleton = mesh.get_editor_property("skeleton")
        if not isinstance(skeleton, unreal.Skeleton):
            raise RuntimeError("Imported mesh has no actual skeleton: " + mesh_path)
        skeleton_path = skeleton.get_path_name()
        if not unreal.EditorAssetLibrary.save_loaded_asset(skeleton, only_if_is_dirty=False):
            raise RuntimeError("Skeleton save failed: " + skeleton_path)
        assert_package(project_root, skeleton_path)
        clips = {}
        for clip, file in inputs["animations"].items():
            sequence, sequence_path = _load_or_import(unreal, project_root, file, role,
                                                      "A_" + clip, unreal.AnimSequence, skeleton)
            length = validate_sequence(sequence, skeleton, sequence_path)
            clips[clip] = {"asset": sequence_path, "duration": length,
                           "file": assert_package(project_root, sequence_path)}
        imported[role] = {"mesh": mesh_path, "skeleton": skeleton_path,
                           "mesh_file": assert_package(project_root, mesh_path), "clips": clips}
    return {"status": "Bestanden", "engine_version": version, "characters": imported,
            "source_validation": source_validation,
            "coverage": "real saved SkeletalMesh/Skeleton/36 AnimSequence packages; exact skeleton and positive clip durations",
            "runtime_animation": "Nicht ausgeführt",
            "manual_animation_quality": "Nicht ausgeführt"}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--preflight", action="store_true")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args(argv)
    default_name = "V78_ANIMATION_SOURCE_PREFLIGHT.json" if args.preflight else "V78_ANIMATION_IMPORT.json"
    report_path = args.report or Path(os.environ.get("ASKIR_V78_ANIMATION_REPORT",
                                      str(args.project_root / "Reports" / default_name)))
    result = {"status": "Nicht ausgeführt", "runtime_animation": "Nicht ausgeführt"}
    code = 0
    try:
        if args.preflight:
            result = source_preflight(args.project_root)
        else:
            try:
                import unreal
            except ImportError:
                result = {"status": "Blockiert", "error": "Unreal 5.6 Python API is unavailable",
                          "runtime_animation": "Nicht ausgeführt"}
                code = 2
            else:
                result = import_assets(unreal, args.project_root)
    except Exception as exc:
        result = {"status": "Fehlgeschlagen", "error": str(exc), "traceback": traceback.format_exc(),
                  "runtime_animation": "Nicht ausgeführt"}
        code = 1
    report_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = report_path.with_suffix(report_path.suffix + ".tmp")
    temporary.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    temporary.replace(report_path)
    print("ASKIR V78 animated import:", result["status"], report_path)
    if code and "unreal" in sys.modules:
        raise RuntimeError("V78 skeletal asset import did not pass: " + result.get("error", "unknown failure"))
    return code


if __name__ == "__main__":
    # UE commandlets own sys.argv; only ASKIR_* environment configuration applies there.
    try:
        import unreal  # noqa: F401
    except ImportError:
        sys.exit(main())
    else:
        main([])
