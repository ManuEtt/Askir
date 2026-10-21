"""Add and validate Rurik in the preserved V80 village exclusively inside UE5.6.

Offline preflight verifies source catalogs and authored bounds, never Unreal
packages or physical accessibility. The editor path saves/reloads the real map,
checks native dialogue/visual references, floor/capsule clearance and navigation,
then revalidates both inherited worlds before publishing a success report.
"""
from __future__ import annotations

import argparse
import ast
import importlib.util
import json
import math
import os
from pathlib import Path
import traceback

ROOT = Path(__file__).resolve().parents[1]
MAP_PATH = "/Game/Askir/Maps/Dornfeld_V80"
OUTSKIRTS_PATH = "/Game/Askir/Maps/Dornfelder_Umland_V80"
MAP_PATHS = (MAP_PATH, OUTSKIRTS_PATH)
RURIK_LABEL = "Dornfeld_Rurik_V83"
RURIK_TAG = "ASKIR_V83_NPC_RURIK"
DIALOGUE_ID = "D_DORNFELD_RURIK"
RURIK_POSITION = (500, -1550, 100)
GUARD_ROOT = "/Game/Askir/CharactersV78/Guard/"
GUARD_MESH = GUARD_ROOT + "SK_AskirGuard"
CLIPS = ("Idle", "Walk", "Run", "Sprint", "Jump", "Dodge", "LightAttack", "HeavyAttack", "Hit", "Death", "Block", "Cast")


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def finite_vector(value, label):
    if not isinstance(value, (list, tuple)) or len(value) != 3 or any(isinstance(number, bool) or not isinstance(number, (int, float)) or not math.isfinite(number) for number in value):
        raise ValueError("Invalid finite 3D coordinates: " + label)
    return list(value)


def navigation_bounds(source):
    """Read the V77 generator's literal nav bounds without executing its code."""
    bounds = []
    for node in ast.walk(ast.parse(source)):
        if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Attribute) or node.func.attr != "create_navigation_bounds":
            continue
        if len(node.args) != 3:
            raise ValueError("Village navigation bounds no longer use the supported native helper")
        vectors = []
        for vector in node.args[1:]:
            if not isinstance(vector, ast.Call) or not isinstance(vector.func, ast.Attribute) or vector.func.attr != "Vector" or vector.keywords:
                raise ValueError("Village navigation bounds must be explicit authored Vector constants")
            vectors.append(finite_vector([ast.literal_eval(value) for value in vector.args], "navigation bounds"))
        bounds.append(vectors)
    if len(bounds) != 1 or any(value <= 0 for value in bounds[0][1]):
        raise ValueError("Exactly one positive authored village navigation envelope is required")
    return bounds[0]


def validate_placement(position, village, nav_center, nav_extent):
    point = finite_vector(position, "Rurik position")
    center = finite_vector(village["ground_center"], "village floor center")
    extent = finite_vector(village["ground_extent"], "village floor extent")
    nav_center = finite_vector(nav_center, "navigation center")
    nav_extent = finite_vector(nav_extent, "navigation extent")
    if any(value <= 0 for value in extent + nav_extent):
        raise ValueError("Floor/navigation extents must be positive")
    if any(abs(point[axis] - center[axis]) > extent[axis] - 100 for axis in (0, 1)):
        raise ValueError("Rurik source position is outside the authored village floor margin")
    if not 40 <= point[2] - center[2] - extent[2] <= 140:
        raise ValueError("Rurik source height is outside the inherited ground-trace envelope")
    if any(abs(point[axis] - nav_center[axis]) > nav_extent[axis] - 100 for axis in range(3)):
        raise ValueError("Rurik source position is outside the authored navigation margin")
    return point


def source_preflight(root=ROOT):
    root = Path(root)
    world = module("_v83_conversation_world_source", root / "Tools/build_dornfeld_v80.py")
    data = world.source_preflight(root)
    village = data["maps"][0]
    nav_center, nav_extent = navigation_bounds((root / "Tools/build_dornfeld_v77.py").read_text(encoding="utf-8"))
    position = validate_placement(RURIK_POSITION, village, nav_center, nav_extent)
    overrides = json.loads((root / "Production/Dialogue/runtime_overrides_v83.json").read_text(encoding="utf-8"))
    matching = [entry for entry in overrides.get("additional", []) if entry.get("id") == DIALOGUE_ID]
    if len(matching) != 1 or matching[0].get("npc") != "Rurik":
        raise ValueError("Exactly one Rurik conversation is required in the native source catalog")
    generator = root / "Tools/generate_conversation_catalog_v83.py"
    generated = [root / "Source/Askir/Dialogue/AskirConversationCatalog.generated.inl", root / "Source/Askir/Dialogue/AskirConversationCatalog.h"]
    if not generator.is_file() or any(not path.is_file() for path in generated):
        raise ValueError("Native conversation generator/catalog files are missing")
    # Unreal embeds Python: sys.executable is the editor, not a Python CLI.
    # Verify in-process so generation never starts another editor instance.
    module("_v83_conversation_catalog_preflight", generator).generate(root, check=True)
    return {"version": "V83", "world_contract_version": "V80", "map_package": MAP_PATH,
            "actor_label": RURIK_LABEL, "stable_tag": RURIK_TAG, "dialogue_id": DIALOGUE_ID,
            "position": position, "navigation_center": nav_center, "navigation_extent": nav_extent,
            "native_catalog_source": "Bestanden", "editor_status": "Nicht ausgeführt", "gameplay_status": "Nicht ausgeführt",
            "coverage": "native catalog freshness and authored floor/navigation source bounds only; physical collision/accessibility untested"}


def tags(actor):
    return {str(value) for value in actor.get_editor_property("tags")}


def identify_rurik(unreal, actors):
    """Conflicting identities fail instead of adopting or replacing unrelated actors."""
    matching = []
    for actor in actors:
        is_npc = isinstance(actor, unreal.AskirNPC)
        conversation = str(actor.get_editor_property("conversation_id")) if is_npc else ""
        identity = actor.get_actor_label() == RURIK_LABEL or RURIK_TAG in tags(actor) or conversation == DIALOGUE_ID
        if identity:
            matching.append(actor)
        elif is_npc and str(actor.get_editor_property("npc_name")) == "Rurik":
            raise RuntimeError("Unrelated existing Rurik NPC preserved; resolve identity explicitly before generation")
    if len(matching) > 1:
        raise RuntimeError("Duplicate Rurik actor/tag/dialogue identities in the real map")
    if not matching:
        return None
    actor = matching[0]
    if not isinstance(actor, unreal.AskirNPC) or actor.get_actor_label() != RURIK_LABEL or RURIK_TAG not in tags(actor) or str(actor.get_editor_property("conversation_id")) != DIALOGUE_ID:
        raise RuntimeError("Existing Rurik identity conflicts with the V83 contract; actor was preserved")
    return actor


def actor_snapshot(actors):
    result = {}
    for actor in actors:
        name = actor.get_path_name()
        if name in result:
            raise RuntimeError("Duplicate actor object path in editor world")
        location, rotation, scale = actor.get_actor_location(), actor.get_actor_rotation(), actor.get_actor_scale3d()
        result[name] = {"label": actor.get_actor_label(), "class": actor.get_class().get_path_name(),
                        "transform": [location.x, location.y, location.z, rotation.pitch, rotation.yaw, rotation.roll, scale.x, scale.y, scale.z]}
    return result


def require_preserved(before, after):
    for path, original in before.items():
        actual = after.get(path)
        if actual is None or actual["label"] != original["label"] or actual["class"] != original["class"] or any(abs(left - right) > .001 for left, right in zip(original["transform"], actual["transform"])):
            raise RuntimeError("Existing actor was removed, replaced, relabelled or moved: " + path)


def reference_package(reference):
    if reference is None:
        raise RuntimeError("Required native visual soft reference is empty")
    if hasattr(reference, "get_path_name"):
        value = reference.get_path_name()
    elif hasattr(reference, "to_string"):
        value = reference.to_string()
    elif hasattr(reference, "get_asset_path_string"):
        value = reference.get_asset_path_string()
    else:
        value = str(reference)
    if not isinstance(value, str) or not value.startswith("/Game/"):
        raise RuntimeError("Cannot resolve a native character asset reference: " + str(value))
    return value.split(".", 1)[0]


def validate_rurik(unreal, actor_api, root, contract):
    actor = identify_rurik(unreal, actor_api.get_all_level_actors())
    if actor is None:
        raise RuntimeError("Rurik was not present after real world reload")
    if str(actor.get_editor_property("npc_name")) != "Rurik" or actor.get_editor_property("hostile"):
        raise RuntimeError("Rurik must be a friendly named native NPC")
    if not unreal.AskirConversationComponent.is_known_dialogue(unreal.Name(DIALOGUE_ID)):
        raise RuntimeError("Compiled native dialogue catalog does not contain Rurik")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    position = actor.get_actor_location()
    spec = json.loads((root / "Production/Levels/dornfeld_outskirts_v80.json").read_text(encoding="utf-8"))["maps"][0]
    validate_placement((position.x, position.y, position.z), spec, contract["navigation_center"], contract["navigation_extent"])
    if not unreal.AskirEditorLevelTools.has_walkable_ground_below(actor):
        raise RuntimeError("Rurik has no actual walkable collision below")
    capsule = actor.get_editor_property("capsule_component")
    extent = unreal.AskirEditorLevelTools.get_player_capsule_extent()
    if not capsule or abs(capsule.get_scaled_capsule_radius() - extent.x) > .01 or abs(capsule.get_scaled_capsule_half_height() - extent.z) > .01:
        raise RuntimeError("Rurik capsule differs from the native clearance-check capsule")
    if not actor.get_actor_enable_collision():
        raise RuntimeError("Rurik actor collision is disabled")
    # Ignore this actor's own capsule, then always restore its actual collision state.
    actor.set_actor_enable_collision(False)
    try:
        if not unreal.AskirEditorLevelTools.is_player_arrival_clear(world, position):
            raise RuntimeError("Rurik capsule overlaps blocking world collision or lacks a walkable floor")
    finally:
        actor.set_actor_enable_collision(True)
    playback = actor.get_editor_property("animation_playback")
    # Native installation synchronously resolves every soft reference first. Python
    # may otherwise expose an unloaded soft UObject reference as None.
    if playback is None or not playback.try_install_visual():
        raise RuntimeError("Rurik native visual installation failed its real skeleton/sequence checks")
    if reference_package(playback.get_editor_property("character_mesh")) != GUARD_MESH:
        raise RuntimeError("Rurik Guard CharacterMesh reference is missing or incorrect")
    clips = playback.get_editor_property("animation_clips")
    if {str(key) for key in clips.keys()} != set(CLIPS) or any(reference_package(clips[unreal.Name(clip)]) != GUARD_ROOT + "A_" + clip for clip in CLIPS):
        raise RuntimeError("Rurik must retain all twelve Guard animation references")
    mesh = actor.get_editor_property("mesh").get_skeletal_mesh_asset()
    if not isinstance(mesh, unreal.SkeletalMesh) or reference_package(mesh) != GUARD_MESH:
        raise RuntimeError("Rurik installed skeletal mesh is not the actual Guard asset")
    labels = {entry.get_actor_label(): entry for entry in actor_api.get_all_level_actors()}
    if "DornfeldPlayerStart" not in labels:
        raise RuntimeError("Preserved village PlayerStart missing")
    start = labels["DornfeldPlayerStart"].get_actor_location()
    economy = module("_v83_conversation_approach", root / "Tools/build_dornfeld_v79.py")
    endpoint = economy.approach_point(unreal, start, position, 190.)
    if not unreal.AskirEditorLevelTools.is_player_arrival_clear(world, endpoint):
        raise RuntimeError("Rurik interaction approach cannot contain the actual native player capsule")
    path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, endpoint)
    if path is None or not path.is_valid() or path.is_partial():
        raise RuntimeError("Rurik has no complete navigation path from village PlayerStart")
    return {"status": "Bestanden", "map_reload": True, "world_asset_type": "UWorld", "map_package": MAP_PATH,
            "actor_label": RURIK_LABEL, "stable_tag": RURIK_TAG, "dialogue_id": DIALOGUE_ID,
            "native_class": actor.get_class().get_path_name(), "npc_name": "Rurik", "unique_actor_count": 1,
            "compiled_dialogue_catalog": "Bestanden", "friendly_npc": "Bestanden", "walkable_ground": "Bestanden",
            "capsule_clearance": "Bestanden", "approach_clearance": "Bestanden", "navigation_path": "Bestanden",
            "character_mesh": GUARD_MESH, "animation_packages": [GUARD_ROOT + "A_" + clip for clip in CLIPS],
            "position": [position.x, position.y, position.z], "capsule_extent": [extent.x, extent.y, extent.z],
            "coverage": "saved native actor identity/dialogue/visual references, real floor and capsule queries and full editor navigation path; conversation gameplay unexecuted"}


def generate(unreal, root=ROOT, validate_only=False):
    root = Path(root)
    contract = source_preflight(root)
    if not unreal.SystemLibrary.get_engine_version().startswith("5.6."):
        raise RuntimeError("Unreal Engine 5.6 required")
    for name in ("AskirNPC", "AskirConversationComponent", "AskirEditorLevelTools"):
        if getattr(unreal, name, None) is None:
            raise RuntimeError("Compile AskirEditor first: " + name)
    builder = module("_v83_conversation_inherited_worlds", root / "Tools/build_dornfeld_v80.py")
    builder.require_world(unreal, root, MAP_PATH)
    actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    before = actor_snapshot(actor_api.get_all_level_actors())
    existing = identify_rurik(unreal, actor_api.get_all_level_actors())
    created = existing is None
    if created:
        if validate_only:
            raise RuntimeError("Validation-only execution cannot add missing Rurik")
        actor = actor_api.spawn_actor_from_class(unreal.AskirNPC, unreal.Vector(*RURIK_POSITION), unreal.Rotator(0, -90, 0))
        if actor is None:
            raise RuntimeError("Native Rurik actor creation failed")
        actor.set_actor_label(RURIK_LABEL)
        actor.set_editor_property("tags", [unreal.Name("ASKIR_V83"), unreal.Name(RURIK_TAG)])
        actor.set_editor_property("npc_name", unreal.Text("Rurik"))
        actor.set_editor_property("conversation_id", unreal.Name(DIALOGUE_ID))
        village = module("_v83_conversation_native_flags", root / "Tools/build_dornfeld_v77.py")
        village.set_boolean_property(actor, "hostile", "bHostile", False)
        actor.set_editor_property("kill_experience", 0)
        actor.set_editor_property("guaranteed_drop", None)
        playback = actor.get_editor_property("animation_playback")
        if playback is None or not playback.configure_character_visual(unreal.Name("Guard")):
            raise RuntimeError("Native Rurik Guard visual configuration failed")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.build_editor_navigation(world):
        raise RuntimeError("Rurik village navigation build failed")
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Rurik real world save failed")
    builder.require_world(unreal, root, MAP_PATH)
    validation = validate_rurik(unreal, actor_api, root, contract)
    require_preserved(before, actor_snapshot(actor_api.get_all_level_actors()))
    # Validation checks the source soft references; persist the installed real mesh too.
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Rurik validated skeletal mesh save failed")
    # Revalidate both inherited worlds after insertion, including all mission/economy
    # paths and reciprocal arrivals. V80's own report is retained in full.
    inherited = builder.generate(unreal, root, validate_only=True)
    builder.require_world(unreal, root, MAP_PATH)
    validation = validate_rurik(unreal, actor_api, root, contract)
    require_preserved(before, actor_snapshot(actor_api.get_all_level_actors()))
    validation.update(created=created, preserved_actor_count=len(before), existing_actors_preserved="Bestanden")
    inherited.update(version="V83", world_contract_version="V80", editor_status="Bestanden", conversation_validation=validation,
                     coverage="two saved/reloaded V80 editor worlds revalidated after native Rurik conversation injection; runtime dialogue/playthrough unexecuted")
    return inherited


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preflight", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args(argv)
    root = Path(os.environ.get("ASKIR_V83_PROJECT", str(ROOT)))
    report = args.report or Path(os.environ.get("ASKIR_V83_CONVERSATION_REPORT", str(root / "Reports/V83/conversations_editor.json")))
    report.parent.mkdir(parents=True, exist_ok=True)
    try:
        if args.preflight:
            result = {"status": "Bestanden", **source_preflight(root)}
        else:
            try:
                import unreal
            except ModuleNotFoundError:
                report.write_text(json.dumps({"version": "V83", "status": "Blockiert", "reason": "Unreal Python unavailable; no maps/assets created", "editor_status": "Nicht ausgeführt", "gameplay_status": "Nicht ausgeführt"}, indent=2), encoding="utf-8")
                return 2
            result = generate(unreal, root, args.validate_only)
            unreal.log("ASKIR_V83_CONVERSATION_WORLD_VALIDATED " + MAP_PATH)
    except Exception as exc:
        report.write_text(json.dumps({"version": "V83", "status": "Fehlgeschlagen", "error": str(exc), "traceback": traceback.format_exc(), "gameplay_status": "Nicht ausgeführt"}, indent=2), encoding="utf-8")
        raise
    report.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
    return 0


if __name__ == "__main__":
    try:
        import unreal
    except ImportError:
        raise SystemExit(main())
    else:
        main([])
