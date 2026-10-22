"""Integrate SQ_003 into the preserved V80 worlds in the real UE5.6 editor.

Offline preflight checks authored sources only. Actual execution imports original
herbs, saves/reloads the two worlds, and verifies native bindings, visibility,
collision, navigation and preservation before reporting editor success.
"""
from __future__ import annotations

import argparse
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
QUEST_ID = "SQ_003"
SOURCE_ID = "SQ003_Meadow_V84"
PATCH_LABEL = "Umland_HealingHerbs_V84"
PATCH_TAG = "ASKIR_V84_HERB_PATCH"
GIVER_TAG = "ASKIR_V84_RURIK_GIVER"
MESH_PACKAGE = "/Game/Askir/V84/Props/SM_AskirHealingHerbs"
REWARD_PACKAGE = "/Game/Askir/V84/Items/item_004"
POSITION = (1100, 900, 100)
VISUAL_OFFSET = (0, 0, -100)
APPROACH = (900, 900, 100)
QUERY_OFFSET = (0, 0, -60)
QUERY_EXTENT = (65, 65, 45)


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def finite_vector(value, label):
    if not isinstance(value, (list, tuple)) or len(value) != 3 or any(isinstance(n, bool) or not isinstance(n, (int, float)) or not math.isfinite(n) for n in value):
        raise ValueError("Invalid finite coordinates: " + label)
    return list(value)


def validate_placement(position, approach, world, nav_center, nav_extent):
    position, approach = finite_vector(position, "herb root"), finite_vector(approach, "herb approach")
    center = finite_vector(world["ground_center"], "floor center")
    extent = finite_vector(world["ground_extent"], "floor extent")
    nav_center, nav_extent = finite_vector(nav_center, "navigation center"), finite_vector(nav_extent, "navigation extent")
    if any(value <= 0 for value in extent + nav_extent):
        raise ValueError("Floor and navigation extents must be positive")
    for point in (position, approach):
        if any(abs(point[i] - center[i]) > extent[i] - 100 for i in (0, 1)):
            raise ValueError("Herb root/approach is outside the authored floor margin")
        if not 40 <= point[2] - center[2] - extent[2] <= 140:
            raise ValueError("Herb root/approach is outside the ground-trace envelope")
        if any(abs(point[i] - nav_center[i]) > nav_extent[i] - 100 for i in range(3)):
            raise ValueError("Herb root/approach is outside the authored navigation margin")
    distance = math.dist(position, approach)
    if not 160 <= distance <= 220:
        raise ValueError("Herb approach must clear the plant and remain within native interaction range")
    return position, approach


def canonical_reward(root):
    items = json.loads((Path(root) / "Production/Items/items.json").read_text(encoding="utf-8"))
    matches = [item for item in items if item.get("id") == "item_004"]
    if len(matches) != 1:
        raise ValueError("Canonical item_004 must resolve uniquely")
    item = matches[0]
    if item.get("category") != "material" or type(item.get("stack_max")) is not int or not 1 <= item["stack_max"] <= 2147483647:
        raise ValueError("SQ_003 reward must retain the canonical stackable material")
    if any(not isinstance(item.get(field), str) or not item[field].strip() for field in ("name", "description")):
        raise ValueError("Canonical SQ_003 reward requires authored name and description")
    if type(item.get("weight")) not in (int, float) or not math.isfinite(item["weight"]) or item["weight"] < 0 or type(item.get("value")) is not int or not 0 <= item["value"] <= 2147483647:
        raise ValueError("Canonical SQ_003 reward weight/value is invalid")
    return {"id": item["id"], "name": item["name"], "description": item["description"], "type": "Misc",
            "stack": item["stack_max"], "weight": item["weight"], "value": item["value"],
            "damage": 0, "heal": 0, "attack_range": 150, "attack_cooldown": .7}


def source_preflight(root=ROOT):
    root = Path(root)
    conversations = module("_v84_herbs_conversation_source", root / "Tools/build_conversations_v83.py")
    conversations.source_preflight(root)
    world = module("_v84_herbs_world_source", root / "Tools/build_dornfeld_v80.py").source_preflight(root)
    spec = next(entry for entry in world["maps"] if entry["package"] == OUTSKIRTS_PATH)
    nav_center, nav_extent = conversations.navigation_bounds((root / "Tools/build_dornfeld_v80.py").read_text(encoding="utf-8"))
    position, approach = validate_placement(POSITION, APPROACH, spec, nav_center, nav_extent)
    quests = json.loads((root / "Production/Quests/sidequests.json").read_text(encoding="utf-8"))
    matches = [entry for entry in quests if entry.get("id") == QUEST_ID]
    if len(matches) != 1:
        raise ValueError("Canonical SQ_003 must resolve uniquely")
    quest = matches[0]
    if quest.get("giver") != "Rurik" or quest.get("title") != "Heilkräuter für Mara" or quest.get("rewards") != {"xp": 150, "gold": 24, "item": "item_004"}:
        raise ValueError("SQ_003 giver/title/rewards differ from the native mission contract")
    objectives = quest.get("objectives", [])
    if len(objectives) != 2 or [(x.get("id"), x.get("type"), x.get("count")) for x in objectives] != [("SQ_003_A", "explore", 1), ("SQ_003_B", "talk", 1)]:
        raise ValueError("SQ_003 must retain one exploration followed by return to Rurik")
    assets = module("_v84_herbs_asset_source", root / "Tools/import_herb_assets_v84.py").source_preflight(root)
    return {"version": "V84", "world_contract_version": "V80", "quest_id": QUEST_ID, "source_id": SOURCE_ID,
            "map_package": MAP_PATH, "patch_map": OUTSKIRTS_PATH, "position": position, "visual_offset": list(VISUAL_OFFSET),
            "approach": approach, "navigation_center": nav_center, "navigation_extent": nav_extent,
            "mesh_package": MESH_PACKAGE, "reward_package": REWARD_PACKAGE, "reward": canonical_reward(root),
            "material_packages": sorted("/Game/Askir/V84/Props/Materials/M_" + name for name in assets["materials"]),
            "giver_tag": GIVER_TAG, "original_asset_sources": assets.get("status", "Bestanden"),
            "editor_status": "Nicht ausgeführt", "gameplay_status": "Nicht ausgeführt",
            "coverage": "canonical quest/reward and original source files plus authored floor/navigation bounds only; physical accessibility untested"}


def tags(actor):
    return {str(value) for value in actor.get_editor_property("tags")}


def identify_patch(unreal, actors):
    matches = []
    for actor in actors:
        native = isinstance(actor, unreal.AskirHerbPatchActor)
        source = str(actor.get_editor_property("source_id")) if native else ""
        if actor.get_actor_label() == PATCH_LABEL or PATCH_TAG in tags(actor) or source == SOURCE_ID:
            matches.append(actor)
    if len(matches) > 1:
        raise RuntimeError("Duplicate herb patch label/tag/source identities; existing actors preserved")
    if not matches:
        return None
    actor = matches[0]
    if not isinstance(actor, unreal.AskirHerbPatchActor) or actor.get_actor_label() != PATCH_LABEL or PATCH_TAG not in tags(actor) or str(actor.get_editor_property("source_id")) != SOURCE_ID:
        raise RuntimeError("Conflicting existing herb patch identity; actor preserved")
    return actor


def verify_vector(actual, expected, label):
    values = finite_vector((actual.x, actual.y, actual.z), label)
    if any(abs(a - b) > .01 for a, b in zip(values, expected)):
        raise RuntimeError("Saved herb geometry differs: " + label)
    return values


def package_file(root, package):
    if not isinstance(package, str) or not package.startswith("/Game/Askir/") or "\\" in package or any(x in ("", ".", "..") for x in package[6:].split("/")):
        raise RuntimeError("Unsafe herb asset package path")
    path = Path(root) / ("Content/" + package[6:] + ".uasset")
    content_root = (Path(root) / "Content").resolve()
    if not path.resolve().is_relative_to(content_root) or path.is_symlink() or not path.is_file() or path.stat().st_size < 128:
        raise RuntimeError("Actual saved herb .uasset is missing: " + package)
    with path.open("rb") as stream:
        if stream.read(4) != b"\xc1\x83\x2a\x9e":
            raise RuntimeError("Saved herb file lacks an actual Unreal editor package header: " + package)
    return str(path)


def reward_asset(unreal, root, create):
    economy = module("_v84_herbs_reward_assets", Path(root) / "Tools/build_dornfeld_v79.py")
    spec = canonical_reward(root)
    if create:
        village = module("_v84_herbs_saved_reward", Path(root) / "Tools/build_dornfeld_v77.py")
        item = economy.economy_assets(unreal, root, {"items": [spec], "asset_root": REWARD_PACKAGE.rsplit("/", 1)[0]}, village)["item_004"]
    else:
        item = unreal.load_asset(REWARD_PACKAGE)
    economy.validate_item(unreal, item, spec)
    package_file(root, REWARD_PACKAGE)
    return item


def validate_patch(unreal, actor_api, root, contract):
    actor = identify_patch(unreal, actor_api.get_all_level_actors())
    if actor is None:
        raise RuntimeError("Saved herb patch did not reload")
    position = verify_vector(actor.get_actor_location(), POSITION, "root")
    verify_vector(actor.get_actor_scale3d(), (1, 1, 1), "root scale")
    visual = actor.get_editor_property("visual")
    mesh = visual.get_editor_property("static_mesh") if visual else None
    if not isinstance(mesh, unreal.StaticMesh) or mesh.get_path_name().split(".", 1)[0] != MESH_PACKAGE:
        raise RuntimeError("Herb patch does not reference the actual imported herb mesh")
    verify_vector(visual.get_editor_property("relative_location"), VISUAL_OFFSET, "visual offset")
    verify_vector(visual.get_editor_property("relative_scale3d"), (1, 1, 1), "visual scale")
    if not actor.get_actor_enable_collision() or abs(float(actor.get_editor_property("interaction_range")) - 240.) > .01:
        raise RuntimeError("Herb interaction collision/range is invalid")
    query = actor.get_editor_property("interaction_query")
    if query is None or query.get_collision_enabled() != unreal.CollisionEnabled.QUERY_ONLY or query.get_collision_response_to_channel(unreal.CollisionChannel.VISIBILITY) != unreal.CollisionResponse.BLOCK or query.get_collision_response_to_channel(unreal.CollisionChannel.PAWN) != unreal.CollisionResponse.IGNORE:
        raise RuntimeError("Native herb query must block visibility and leave player movement clear")
    verify_vector(query.get_editor_property("relative_location"), QUERY_OFFSET, "interaction query offset")
    verify_vector(query.get_unscaled_box_extent(), QUERY_EXTENT, "interaction query extent")
    verify_vector(query.get_editor_property("relative_scale3d"), (1, 1, 1), "interaction query scale")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.has_walkable_ground_below(actor):
        raise RuntimeError("Herb patch has no actual walkable ground")
    actor.set_actor_enable_collision(False)
    try:
        if not unreal.AskirEditorLevelTools.is_player_arrival_clear(world, actor.get_actor_location()):
            raise RuntimeError("Herb site capsule overlaps existing world collision or lacks a floor")
    finally:
        actor.set_actor_enable_collision(True)
    endpoint = unreal.Vector(*contract["approach"])
    if not unreal.AskirEditorLevelTools.is_player_arrival_clear(world, endpoint):
        raise RuntimeError("Herb interaction approach cannot contain the actual player capsule")
    labels = {entry.get_actor_label(): entry for entry in actor_api.get_all_level_actors()}
    if "UmlandPlayerStart" not in labels:
        raise RuntimeError("Preserved countryside PlayerStart is missing")
    start = labels["UmlandPlayerStart"].get_actor_location()
    path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, endpoint)
    if path is None or not path.is_valid() or path.is_partial():
        raise RuntimeError("Herb interaction approach has no complete navigation path")
    query_center = query.get_world_location()
    verify_vector(query_center, tuple(a + b for a, b in zip(POSITION, QUERY_OFFSET)), "interaction query world center")
    if not unreal.AskirEditorLevelTools.has_visibility_line_to_actor(world, endpoint + unreal.Vector(0, 0, 50), query_center, actor):
        raise RuntimeError("Actual approach-to-herb visibility trace did not hit the native interaction actor")
    packages = [MESH_PACKAGE, REWARD_PACKAGE]
    for slot in mesh.get_editor_property("static_materials"):
        material = slot.get_editor_property("material_interface")
        if material is None:
            raise RuntimeError("Saved herb mesh has an empty material slot")
        packages.append(material.get_path_name().split(".", 1)[0])
    if sorted(packages[2:]) != contract["material_packages"]:
        raise RuntimeError("Saved herb mesh must retain exactly the six original managed material references")
    for package in packages:
        package_file(root, package)
    reward_asset(unreal, root, False)
    extent = unreal.AskirEditorLevelTools.get_player_capsule_extent()
    return {"status": "Bestanden", "map_reload": True, "world_asset_type": "UWorld", "map_package": OUTSKIRTS_PATH,
            "native_class": actor.get_class().get_path_name(), "actor_label": PATCH_LABEL, "stable_tag": PATCH_TAG,
            "source_id": SOURCE_ID, "unique_actor_count": 1, "position": position, "visual_offset": list(VISUAL_OFFSET),
            "query_offset": list(QUERY_OFFSET), "query_extent": list(QUERY_EXTENT),
            "approach": list(APPROACH), "mesh_package": MESH_PACKAGE, "reward_package": REWARD_PACKAGE,
            "reward_item_id": "item_004", "canonical_reward": "Bestanden", "interaction_query": "Bestanden",
            "visibility_trace": "Bestanden", "walkable_ground": "Bestanden", "capsule_clearance": "Bestanden",
            "approach_clearance": "Bestanden", "navigation_path": "Bestanden", "asset_package_files": sorted(set(packages)),
            "capsule_extent": [extent.x, extent.y, extent.z]}


def validate_giver(unreal, actor_api, conversations):
    actors = actor_api.get_all_level_actors()
    actor = conversations.identify_rurik(unreal, actors)
    marked = [entry for entry in actors if GIVER_TAG in tags(entry)]
    if actor is None or len(marked) != 1 or marked[0] != actor or actor.get_editor_property("hostile"):
        raise RuntimeError("Exactly the existing friendly Rurik must own the trusted SQ_003 giver marker")
    return {"status": "Bestanden", "map_reload": True, "map_package": MAP_PATH, "actor_label": conversations.RURIK_LABEL,
            "dialogue_id": conversations.DIALOGUE_ID, "giver_tag": GIVER_TAG, "unique_actor_count": 1}


def generate(unreal, root=ROOT, validate_only=False):
    root = Path(root)
    contract = source_preflight(root)
    if not unreal.SystemLibrary.get_engine_version().startswith("5.6."):
        raise RuntimeError("Unreal Engine 5.6 required")
    for name in ("AskirHerbPatchActor", "AskirHerbMissionComponent", "AskirEditorLevelTools"):
        if getattr(unreal, name, None) is None:
            raise RuntimeError("Compile AskirEditor first: " + name)
    world_builder = module("_v84_herbs_existing_worlds", root / "Tools/build_dornfeld_v80.py")
    conversations = module("_v84_herbs_existing_rurik", root / "Tools/build_conversations_v83.py")
    actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    before = {}
    for package in MAP_PATHS:
        world_builder.require_world(unreal, root, package)
        before[package] = conversations.actor_snapshot(actor_api.get_all_level_actors())
    asset_importer = module("_v84_original_herb_import", root / "Tools/import_herb_assets_v84.py")
    if not validate_only:
        imported = asset_importer.import_assets(unreal, root)
        if imported.get("status") != "Bestanden":
            raise RuntimeError("Original herb mesh import did not complete")
    if asset_importer.validate_imported_assets(unreal, root).get("status") != "Bestanden":
        raise RuntimeError("Saved original herb mesh/material validation did not complete")
    reward_asset(unreal, root, not validate_only)
    world_builder.require_world(unreal, root, MAP_PATH)
    actor = conversations.identify_rurik(unreal, actor_api.get_all_level_actors())
    if actor is None:
        raise RuntimeError("V83 Rurik must exist before SQ_003 integration")
    if GIVER_TAG not in tags(actor):
        if validate_only:
            raise RuntimeError("Validation-only execution cannot add the missing Rurik quest marker")
        actor.set_editor_property("tags", list(actor.get_editor_property("tags")) + [unreal.Name(GIVER_TAG)])
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Rurik SQ_003 giver world save failed")
    world_builder.require_world(unreal, root, MAP_PATH)
    giver_validation = validate_giver(unreal, actor_api, conversations)
    conversations.require_preserved(before[MAP_PATH], conversations.actor_snapshot(actor_api.get_all_level_actors()))
    world_builder.require_world(unreal, root, OUTSKIRTS_PATH)
    actor = identify_patch(unreal, actor_api.get_all_level_actors())
    created = actor is None
    if created:
        if validate_only:
            raise RuntimeError("Validation-only execution cannot create the missing herb patch")
        actor = actor_api.spawn_actor_from_class(unreal.AskirHerbPatchActor, unreal.Vector(*POSITION), unreal.Rotator(0, 0, 0))
        if actor is None:
            raise RuntimeError("Native herb patch spawn failed")
        actor.set_actor_label(PATCH_LABEL)
        actor.set_editor_property("tags", [unreal.Name("ASKIR_V84"), unreal.Name(PATCH_TAG)])
        actor.set_editor_property("source_id", unreal.Name(SOURCE_ID))
        actor.set_editor_property("interaction_range", 240.)
        visual = actor.get_editor_property("visual")
        if not visual.set_static_mesh(unreal.load_asset(MESH_PACKAGE)):
            raise RuntimeError("Native herb visual did not accept the actual mesh")
        visual.set_editor_property("relative_location", unreal.Vector(*VISUAL_OFFSET))
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.build_editor_navigation(world) or not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Herb countryside navigation/save failed")
    world_builder.require_world(unreal, root, OUTSKIRTS_PATH)
    patch_validation = validate_patch(unreal, actor_api, root, contract)
    conversations.require_preserved(before[OUTSKIRTS_PATH], conversations.actor_snapshot(actor_api.get_all_level_actors()))
    # Revalidate the complete inherited mission/economy/travel contract and Rurik
    # after insertion. No new world is substituted for the preserved maps.
    inherited = conversations.generate(unreal, root, validate_only=True)
    for package in MAP_PATHS:
        world_builder.require_world(unreal, root, package)
        conversations.require_preserved(before[package], conversations.actor_snapshot(actor_api.get_all_level_actors()))
        if package == MAP_PATH:
            giver_validation = validate_giver(unreal, actor_api, conversations)
        else:
            patch_validation = validate_patch(unreal, actor_api, root, contract)
    patch_validation.update(created=created, preserved_actor_count=len(before[OUTSKIRTS_PATH]), existing_actors_preserved="Bestanden")
    giver_validation.update(preserved_actor_count=len(before[MAP_PATH]), existing_actors_preserved="Bestanden")
    world_builder.require_world(unreal, root, MAP_PATH)
    return {"version": "V84", "world_contract_version": "V80", "status": "Bestanden", "editor_status": "Bestanden",
            "gameplay_status": "Nicht ausgeführt", "inherited_v83": inherited, "herb_validation": patch_validation,
            "giver_validation": giver_validation,
            "coverage": "two real saved/reloaded V80 worlds with preserved V83 Rurik, original herb mesh/query/navigation and canonical reward; SQ_003 playthrough unexecuted"}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preflight", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args(argv)
    root = Path(os.environ.get("ASKIR_V84_PROJECT", str(ROOT)))
    report = args.report or Path(os.environ.get("ASKIR_V84_HERB_REPORT", str(root / "Reports/V84/herbs_editor.json")))
    report.parent.mkdir(parents=True, exist_ok=True)
    try:
        if args.preflight:
            result = {"status": "Bestanden", **source_preflight(root)}
        else:
            try:
                import unreal
            except ModuleNotFoundError:
                report.write_text(json.dumps({"version": "V84", "status": "Blockiert", "reason": "Unreal Python unavailable; no maps/assets created", "editor_status": "Nicht ausgeführt", "gameplay_status": "Nicht ausgeführt"}, indent=2), encoding="utf-8")
                return 2
            result = generate(unreal, root, args.validate_only or os.environ.get("ASKIR_V84_VALIDATE_ONLY") == "1")
            unreal.log("ASKIR_V84_HERB_WORLD_VALIDATED " + OUTSKIRTS_PATH)
    except Exception as exc:
        report.write_text(json.dumps({"version": "V84", "status": "Fehlgeschlagen", "error": str(exc), "traceback": traceback.format_exc(), "gameplay_status": "Nicht ausgeführt"}, indent=2), encoding="utf-8")
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
