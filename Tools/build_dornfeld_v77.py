"""Run with UE5.6 PythonScript commandlet after compiling AskirEditor.

Creates actual packages, refuses to overwrite an existing map, and reloads it before
validating. This is a playable integration blockout candidate, not a finished level.
External runner requires the success report as well as the engine exit code.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import traceback
import wave

MAP_PATH = "/Game/Askir/Maps/Dornfeld_V77"
ASSET_ROOT = "/Game/Askir/V77"
SHARDS = (
    ("Dornfeld_Shard_North", "Brunnenscherbe: Nord", (-850, 150, 65)),
    ("Dornfeld_Shard_East", "Brunnenscherbe: Ost", (800, 700, 65)),
    ("Dornfeld_Shard_Well", "Brunnenscherbe: Brunnen", (-160, 1100, 65)),
)
GUARDIANS = (
    ("Dornfeld_Guardian_1", (-380, 1700, 96)),
    ("Dornfeld_Guardian_2", (380, 1800, 96)),
)
CHEST_ID = "Dornfeld_CellarChest"
REQUIRED_CLASSES = (
    "AskirGameMode", "AskirCharacter", "AskirDornfeldEventActor",
    "AskirDornfeldMissionRelay", "AskirLootPickup", "AskirLootChest",
    "AskirItemData", "AskirNPC", "AskirEditorLevelTools",
)
AUDIO_CUES = ("Shard", "Chest", "QuestAccepted", "QuestCompleted", "MeleeSwing", "MeleeHit", "DungeonAmbient")
MATERIAL_NAMES = ("M_DornfeldStone", "M_DornfeldEarth", "M_DornfeldTimber", "M_DornfeldRoof", "M_DornfeldShard", "M_DornfeldMara")


def assert_saved_package(project_root, package):
    if not package.startswith("/Game/"):
        raise RuntimeError("Expected generated project package: " + package)
    path = Path(project_root) / "Content" / (package[len("/Game/"):] + ".uasset")
    if not path.is_file() or path.stat().st_size < 128:
        raise RuntimeError("Editor asset save returned but actual package missing/empty: " + str(path))
    return str(path)


def set_boolean_property(obj, snake_name, native_name, value):
    # Unreal's Python naming strips a leading 'b' for bools; native names remain accepted aliases.
    errors = []
    for name in (snake_name, native_name, "b_" + snake_name):
        try:
            obj.set_editor_property(name, value)
            return
        except Exception as exc:
            errors.append(str(exc))
    raise RuntimeError("Cannot set required reflected boolean " + native_name + ": " + "; ".join(errors))


def design_contract():
    """Pure data checked by local tests; this does not produce or validate a map."""
    ids = [entry[0] for entry in SHARDS] + [entry[0] for entry in GUARDIANS] + [CHEST_ID]
    if len(ids) != len(set(ids)):
        raise ValueError("Every persistent mission source must have a distinct ID")
    return {"map_package": MAP_PATH, "shard_ids": [entry[0] for entry in SHARDS],
            "guardian_ids": [entry[0] for entry in GUARDIANS], "chest_id": CHEST_ID}


def startup_config_text(original):
    """Update only map keys; preserve Unreal's duplicate array entries and other sections."""
    section = "[/Script/EngineSettings.GameMapsSettings]"
    lines = original.splitlines(keepends=True)
    ending = "\r\n" if "\r\n" in original else "\n"
    output = []
    in_section = False
    saw_section = False
    written = set()
    keys = ("GameDefaultMap", "EditorStartupMap")

    def add_missing():
        for key in keys:
            if key not in written:
                if output and not output[-1].endswith(("\r", "\n")):
                    output[-1] += ending
                output.append(key + "=" + MAP_PATH + ending)
                written.add(key)

    for line in lines:
        stripped = line.strip()
        if stripped.startswith("[") and stripped.endswith("]"):
            if in_section:
                add_missing()
            in_section = stripped == section
            saw_section |= in_section
        if in_section and "=" in line and not stripped.startswith((";", "#")):
            key = line.split("=", 1)[0].strip()
            if key in keys:
                if key not in written:
                    output.append(key + "=" + MAP_PATH + ending)
                    written.add(key)
                continue
        output.append(line)
    if not saw_section:
        if output and not output[-1].endswith(("\r", "\n")):
            output[-1] += ending
        output.append(ending + section + ending)
    add_missing()
    return "".join(output)


def configure_startup_maps(project_root, validation):
    """Never point startup at a promised asset. Call only after actual save/load validation."""
    required = ("spawn_collision_traces", "relay_source_references", "navigation_brush_geometry", "native_game_mode")
    if validation.get("map_reload") is not True or any(validation.get(key) != "Bestanden" for key in required):
        raise RuntimeError("Default map config requires successful saved-map reload and validation")
    project_root = Path(project_root)
    map_file = project_root / "Content/Askir/Maps/Dornfeld_V77.umap"
    if not map_file.is_file() or map_file.stat().st_size < 1024:
        raise RuntimeError("Default map config requires an existing real .umap file")
    config = project_root / "Config/DefaultEngine.ini"
    if not config.is_file():
        raise RuntimeError("DefaultEngine.ini missing; refusing to replace missing configuration")
    with config.open("r", encoding="utf-8-sig", newline="") as handle:
        original = handle.read()
    updated = startup_config_text(original)
    temporary = config.with_suffix(".ini.v77.tmp")
    with temporary.open("w", encoding="utf-8", newline="") as handle:
        handle.write(updated)
    temporary.replace(config)
    return {"status": "Bestanden", "config": str(config), "game_default_map": MAP_PATH, "editor_startup_map": MAP_PATH}


def generate(unreal, project_root: Path):
    version = unreal.SystemLibrary.get_engine_version()
    if not version.startswith("5.6."):
        raise RuntimeError("UE5.6 required; editor reported " + version)
    for name in REQUIRED_CLASSES:
        if getattr(unreal, name, None) is None:
            raise RuntimeError("Native class unavailable; compile AskirEditor first: " + name)
    assets = unreal.EditorAssetLibrary
    if assets.does_asset_exist(MAP_PATH):
        raise RuntimeError("Existing map preserved; refusing overwrite: " + MAP_PATH)
    for name in ("Cube", "Sphere", "Cylinder", "Cone"):
        if not unreal.load_asset("/Engine/BasicShapes/" + name + "." + name):
            raise RuntimeError("Required licensed engine primitive unavailable: " + name)

    actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    created_assets = []
    created_actors = []

    def spawn(cls, label, xyz, rotation=(0, 0, 0)):
        actor = actor_api.spawn_actor_from_class(cls, unreal.Vector(*xyz), unreal.Rotator(*rotation))
        if actor is None:
            raise RuntimeError("Actor creation failed: " + label)
        actor.set_actor_label(label)
        actor.set_editor_property("tags", [unreal.Name("ASKIR_V77")])
        created_actors.append(label)
        return actor

    def material(name, color, emissive=False):
        path = ASSET_ROOT + "/Materials/" + name
        if assets.does_asset_exist(path):
            result = unreal.load_asset(path)
            if not isinstance(result, unreal.Material):
                raise RuntimeError("Unexpected existing asset type: " + path)
            return result
        result = asset_tools.create_asset(name, ASSET_ROOT + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
        if not result:
            raise RuntimeError("Material creation failed: " + path)
        expression = unreal.MaterialEditingLibrary.create_material_expression(result, unreal.MaterialExpressionConstant3Vector, -300, 0)
        expression.set_editor_property("constant", unreal.LinearColor(*color, 1))
        prop = unreal.MaterialProperty.MP_EMISSIVE_COLOR if emissive else unreal.MaterialProperty.MP_BASE_COLOR
        if not unreal.MaterialEditingLibrary.connect_material_property(expression, "", prop):
            raise RuntimeError("Material connection failed: " + path)
        unreal.MaterialEditingLibrary.recompile_material(result)
        if not assets.save_loaded_asset(result, only_if_is_dirty=False):
            raise RuntimeError("Material save failed: " + path)
        created_assets.append(path)
        return result

    stone = material("M_DornfeldStone", (.13, .16, .19))
    earth = material("M_DornfeldEarth", (.085, .12, .06))
    timber = material("M_DornfeldTimber", (.18, .085, .035))
    roof = material("M_DornfeldRoof", (.18, .055, .05))
    crystal = material("M_DornfeldShard", (.0, .6, 1.0), emissive=True)
    mara_mat = material("M_DornfeldMara", (.25, .38, .2))
    mesh = {name: unreal.load_asset("/Engine/BasicShapes/" + name + "." + name)
            for name in ("Cube", "Sphere", "Cylinder", "Cone")}

    def block(label, xyz, scale, mat=stone, shape="Cube", collides=True):
        actor = spawn(unreal.StaticMeshActor, label, xyz)
        component = actor.get_editor_property("static_mesh_component")
        component.set_static_mesh(mesh[shape])
        component.set_material(0, mat)
        component.set_collision_profile_name("BlockAll" if collides else "NoCollision")
        actor.set_actor_scale3d(unreal.Vector(*scale))
        return actor

    def item(name, display, kind, value=0):
        path = ASSET_ROOT + "/Items/" + name
        result = unreal.load_asset(path) if assets.does_asset_exist(path) else None
        if result is None:
            factory = unreal.DataAssetFactory()
            factory.set_editor_property("data_asset_class", unreal.AskirItemData)
            result = asset_tools.create_asset(name, ASSET_ROOT + "/Items", unreal.AskirItemData, factory)
            if not result:
                raise RuntimeError("Item DataAsset creation failed: " + path)
            result.set_editor_property("item_id", unreal.Name(name))
            result.set_editor_property("display_name", unreal.Text(display))
            result.set_editor_property("description", unreal.Text("Dornfeld: Das Flüstern im Brunnen"))
            result.set_editor_property("item_type", kind)
            result.set_editor_property("max_stack_size", 1)
            result.set_editor_property("value", value)
            if not assets.save_loaded_asset(result, only_if_is_dirty=False):
                raise RuntimeError("Item save failed: " + path)
            created_assets.append(path)
        if not isinstance(result, unreal.AskirItemData) or str(result.get_editor_property("item_id")) != name:
            raise RuntimeError("Existing item conflicts with generator contract: " + path)
        return result

    def import_audio():
        source_root = project_root / "ArtSource/Audio_V77"
        imported = {}
        tasks = []
        for cue in AUDIO_CUES:
            name = "S_" + cue
            path = ASSET_ROOT + "/Audio/" + name
            if assets.does_asset_exist(path):
                sound = unreal.load_asset(path)
                if not isinstance(sound, unreal.SoundWave):
                    raise RuntimeError("Existing audio package is not SoundWave: " + path)
                imported[cue] = sound
                continue
            source = source_root / (name + ".wav")
            if not source.is_file():
                raise RuntimeError("Required original audio source missing: " + str(source))
            with wave.open(str(source), "rb") as reader:
                if reader.getnframes() <= 0 or reader.getnchannels() not in (1, 2) or reader.getsampwidth() != 2:
                    raise RuntimeError("Expected nonempty original 16-bit PCM audio: " + str(source))
            task = unreal.AssetImportTask()
            task.set_editor_property("filename", str(source))
            task.set_editor_property("destination_path", ASSET_ROOT + "/Audio")
            task.set_editor_property("destination_name", name)
            task.set_editor_property("automated", True)
            task.set_editor_property("replace_existing", False)
            task.set_editor_property("save", True)
            tasks.append(task)
        if tasks:
            asset_tools.import_asset_tasks(tasks)
        for cue in AUDIO_CUES:
            path = ASSET_ROOT + "/Audio/S_" + cue
            sound = unreal.load_asset(path)
            if not isinstance(sound, unreal.SoundWave):
                raise RuntimeError("SoundWave import did not create valid real asset: " + path)
            if cue == "DungeonAmbient":
                sound.set_editor_property("looping", True)
            if not assets.save_loaded_asset(sound, only_if_is_dirty=False):
                raise RuntimeError("Imported SoundWave could not be saved: " + path)
            imported[cue] = sound
            if path not in created_assets:
                created_assets.append(path)
        return imported

    shard_items = [item(source[0], source[1], unreal.AskirItemType.QUEST) for source in SHARDS]
    relic = item("Dornfeld_WellRelic", "Verwittertes Brunnensiegel", unreal.AskirItemType.QUEST)
    sounds = import_audio()
    if not unreal.EditorLevelLibrary.new_level(MAP_PATH):
        raise RuntimeError("Could not create real editor level: " + MAP_PATH)

    block("Dornfeld_Ground", (0, 500, -50), (48, 64, 1), earth)
    block("Dornfeld_MainRoad", (0, 350, 2), (4.5, 47, .04), stone)
    # Open central corridor. Buildings are collision solid; their doors are visual facades.
    for number, (x, y) in enumerate(((-1350, -1050), (1250, -950), (-1450, 200), (1450, 350))):
        block("House_%d" % number, (x, y, 170), (6, 6, 3.4), timber)
        block("Roof_%d" % number, (x, y, 390), (7.6, 7.6, 1.4), roof, "Cone")
        block("Door_%d" % number, (x - (305 if x > 0 else -305), y, 90), (.1, 1.1, 1.8), stone, collides=False)
    # Ring segments leave a central gap; each segment has explicit collision.
    for index, (x, y, scale) in enumerate(((100, 1180, (2.2, .35, 1.2)), (100, 920, (2.2, .35, 1.2)),
                                          (-30, 1050, (.35, 2.2, 1.2)), (230, 1050, (.35, 2.2, 1.2)))):
        block("WellStone_%d" % index, (x, y, 60), scale, stone)
    # Cellar entrance is a ramp into an above-ground covered chamber; no fake transition marker.
    block("Cellar_Floor", (0, 2390, 10), (8, 8, .2), stone)
    for label, xyz, scale in (
        ("West", (-420, 2390, 145), (.35, 8.4, 2.9)),
        ("East", (420, 2390, 145), (.35, 8.4, 2.9)),
        ("Back", (0, 2810, 145), (8.4, .35, 2.9)),
        ("Roof", (0, 2390, 310), (8.7, 8.7, .3)),
    ):
        block("Cellar_" + label, xyz, scale)

    start = spawn(unreal.PlayerStart, "DornfeldPlayerStart", (0, -1700, 100), (0, 90, 0))
    mara = spawn(unreal.AskirDornfeldEventActor, "Mara", (-240, -900, 90))
    mara.set_editor_property("event", unreal.AskirDornfeldEvent.ELDER_DIALOGUE)
    mara.set_editor_property("persistent_event_id", unreal.Name("Dornfeld_Mara"))
    mara.set_editor_property("interaction_range", 250.0)
    mara_visual = mara.get_editor_property("visual")
    mara_visual.set_static_mesh(mesh["Cylinder"])
    mara_visual.set_material(0, mara_mat)
    mara_visual.set_relative_scale3d(unreal.Vector(.65, .65, 1.8))

    sources = {}
    for spec, data in zip(SHARDS, shard_items):
        event_id, _, xyz = spec
        pickup = spawn(unreal.AskirLootPickup, event_id, xyz)
        pickup.set_editor_property("item", data)
        pickup.set_editor_property("quantity", 1)
        pickup.set_editor_property("persistent_loot_id", unreal.Name(event_id))
        pickup.set_editor_property("dornfeld_shard_id", unreal.Name(event_id))
        visual = pickup.get_editor_property("visual")
        visual.set_static_mesh(mesh["Sphere"])
        visual.set_material(0, crystal)
        visual.set_relative_scale3d(unreal.Vector(.55, .35, .45))
        sources[event_id] = (pickup, "shard_pickup")

    for event_id, xyz in GUARDIANS:
        guardian = spawn(unreal.AskirNPC, event_id, xyz, (0, -90, 0))
        guardian.set_editor_property("npc_name", unreal.Text("Schattenwächter"))
        set_boolean_property(guardian, "hostile", "bHostile", True)
        guardian.set_editor_property("kill_experience", 0)  # mission completion grants exactly 180 XP
        guardian.set_editor_property("detection_radius", 1100.0)
        guardian.set_editor_property("max_chase_distance", 1800.0)
        guardian.set_editor_property("attack_damage", 8.0)
        guardian.set_editor_property("attack_interval", 1.5)
        sources[event_id] = (guardian, "guardian_actor")

    chest = spawn(unreal.AskirLootChest, CHEST_ID, (0, 2580, 65))
    chest.set_editor_property("persistent_loot_id", unreal.Name(CHEST_ID))
    chest.set_editor_property("dornfeld_chest_id", unreal.Name(CHEST_ID))
    entry = unreal.AskirChestEntry()
    entry.set_editor_property("item", relic)
    entry.set_editor_property("quantity", 1)
    chest.set_editor_property("contents", [entry])
    chest_visual = chest.get_editor_property("visual")
    chest_visual.set_static_mesh(mesh["Cube"])
    chest_visual.set_material(0, timber)
    chest_visual.set_relative_scale3d(unreal.Vector(1.4, .9, 1.1))
    sources[CHEST_ID] = (chest, "mission_chest")

    for event_id, (source, property_name) in sources.items():
        relay = spawn(unreal.AskirDornfeldMissionRelay, "Relay_" + event_id, (0, 0, 0))
        relay.set_editor_property("event_id", unreal.Name(event_id))
        relay.set_editor_property(property_name, source)

    sun = spawn(unreal.DirectionalLight, "Dornfeld_Moonlight", (0, 0, 1500), (-45, -30, 0))
    sun.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sun.light_component.set_editor_property("intensity", 2.5)
    sky = spawn(unreal.SkyLight, "Dornfeld_SkyLight", (0, 0, 1000))
    sky.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property("intensity", .8)
    # SkyLight requires a source environment for capture. Engine atmospheric sky works in cooked builds.
    spawn(unreal.SkyAtmosphere, "Dornfeld_SkyAtmosphere", (0, 0, 0))
    spawn(unreal.ExponentialHeightFog, "Dornfeld_Fog", (0, 0, 20))
    lamp = spawn(unreal.PointLight, "Dornfeld_CellarLamp", (0, 2390, 220))
    lamp.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    lamp.light_component.set_editor_property("intensity", 1500.0)
    lamp.light_component.set_editor_property("attenuation_radius", 700.0)
    ambience = spawn(unreal.AmbientSound, "Dornfeld_Ambience", (0, 2390, 150))
    ambience.audio_component.set_sound(sounds["DungeonAmbient"])
    ambience.audio_component.set_editor_property("auto_activate", True)
    ambience.audio_component.set_editor_property("allow_spatialization", True)
    ambience.audio_component.set_volume_multiplier(.15)

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.configure_editor_game_mode(world, unreal.AskirGameMode, True):
        raise RuntimeError("Could not set native AskirGameMode")
    nav = unreal.AskirEditorLevelTools.create_navigation_bounds(world, unreal.Vector(0, 450, 150), unreal.Vector(2350, 3100, 350))
    if nav is None:
        raise RuntimeError("Real navigation brush creation failed")
    nav.set_actor_label("Dornfeld_NavigationBounds")
    nav.set_editor_property("tags", [unreal.Name("ASKIR_V77")])
    created_actors.append("Dornfeld_NavigationBounds")
    if not unreal.AskirEditorLevelTools.build_editor_navigation(world):
        raise RuntimeError("Navigation build did not create navigation data")
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Could not save real .umap package")
    map_file = project_root / "Content/Askir/Maps/Dornfeld_V77.umap"
    if not map_file.is_file() or map_file.stat().st_size < 1024:
        raise RuntimeError("Editor reported save success but .umap missing or implausibly small")
    if not unreal.EditorLevelLibrary.load_level(MAP_PATH):
        raise RuntimeError("Saved .umap could not be reloaded")
    validation = validate_saved_level(unreal, actor_api, project_root)
    startup_config = configure_startup_maps(project_root, validation)
    return {"status": "Bestanden", "coverage": "editor generation/save/reload and structural validation",
            "engine_version": version, "map_package": MAP_PATH, "map_file": str(map_file),
            "created_actors": created_actors, "created_assets": created_assets,
            "validation": validation, "startup_map_config": startup_config, "gameplay_status": "Nicht ausgeführt",
            "remaining": ["runtime movement/combat/mission/save-load and manual visual QA",
                          "rigged models, animation, audio and production environment art"]}


def validate_saved_level(unreal, actor_api, project_root):
    actors = actor_api.get_all_level_actors()
    labels = {actor.get_actor_label(): actor for actor in actors}
    required = ["DornfeldPlayerStart", "Dornfeld_Ground", "Mara", "Dornfeld_NavigationBounds",
                "Dornfeld_Moonlight", "Dornfeld_SkyLight", "Dornfeld_Ambience", CHEST_ID]
    required += [source[0] for source in SHARDS] + [source[0] for source in GUARDIANS]
    required += ["Relay_" + name for name in design_contract()["shard_ids"] + design_contract()["guardian_ids"] + [CHEST_ID]]
    for label in required:
        if label not in labels:
            raise RuntimeError("Saved map lost required actor: " + label)
    if not unreal.AskirEditorLevelTools.has_walkable_ground_below(labels["DornfeldPlayerStart"]):
        raise RuntimeError("Collision trace found no walkable ground below PlayerStart")
    for label in [spec[0] for spec in GUARDIANS]:
        if not unreal.AskirEditorLevelTools.has_walkable_ground_below(labels[label]):
            raise RuntimeError("Guardian spawn is not above walkable collision: " + label)
    nav_bounds = labels["Dornfeld_NavigationBounds"].get_actor_bounds(False)
    extent = nav_bounds[1]
    if extent.x < 2300 or extent.y < 3000 or extent.z < 300:
        raise RuntimeError("Navigation bounds have missing/incorrect brush geometry")
    ids = set()
    for name in design_contract()["shard_ids"] + design_contract()["guardian_ids"] + [CHEST_ID]:
        relay = labels["Relay_" + name]
        if str(relay.get_editor_property("event_id")) != name:
            raise RuntimeError("Mission relay ID corrupted on reload: " + name)
        refs = [relay.get_editor_property(prop) for prop in ("shard_pickup", "guardian_actor", "mission_chest")]
        refs = [ref for ref in refs if ref is not None]
        if len(refs) != 1 or refs[0].get_actor_label() != name:
            raise RuntimeError("Mission relay must reference exactly its matching source: " + name)
        if name in ids:
            raise RuntimeError("Duplicate persistent mission ID: " + name)
        ids.add(name)
    for source in SHARDS:
        actor = labels[source[0]]
        if str(actor.get_editor_property("dornfeld_shard_id")) != source[0] or actor.get_editor_property("item") is None:
            raise RuntimeError("Shard mission ID/item reference corrupted")
    chest = labels[CHEST_ID]
    if str(chest.get_editor_property("dornfeld_chest_id")) != CHEST_ID or len(chest.get_editor_property("contents")) != 1:
        raise RuntimeError("Mission chest lost gating ID or contents")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.configure_editor_game_mode(world, unreal.AskirGameMode, False):
        raise RuntimeError("Saved map does not select native AskirGameMode")
    for package in (ASSET_ROOT + "/Items/" + name for name in design_contract()["shard_ids"] + ["Dornfeld_WellRelic"]):
        if not unreal.EditorAssetLibrary.does_asset_exist(package) or unreal.load_asset(package) is None:
            raise RuntimeError("Saved map required item package cannot load: " + package)
        assert_saved_package(project_root, package)
    packages = [ASSET_ROOT + "/Items/" + name for name in design_contract()["shard_ids"] + ["Dornfeld_WellRelic"]]
    for name in MATERIAL_NAMES:
        package = ASSET_ROOT + "/Materials/" + name
        if not isinstance(unreal.load_asset(package), unreal.Material):
            raise RuntimeError("Saved map expected Material cannot load: " + package)
        assert_saved_package(project_root, package)
        packages.append(package)
    for cue in AUDIO_CUES:
        path = ASSET_ROOT + "/Audio/S_" + cue
        if not isinstance(unreal.load_asset(path), unreal.SoundWave):
            raise RuntimeError("Saved map expected native audio cue cannot load: " + path)
        assert_saved_package(project_root, path)
        packages.append(path)
    if labels["Dornfeld_Ambience"].audio_component.get_editor_property("sound") is None:
        raise RuntimeError("Saved map ambient actor lost SoundWave reference")
    return {"required_actor_count": len(required), "actor_count": len(actors), "map_reload": True,
            "spawn_collision_traces": "Bestanden", "relay_source_references": "Bestanden",
            "navigation_brush_geometry": "Bestanden", "native_game_mode": "Bestanden",
            "audio_asset_references": "Bestanden", "asset_package_files": packages,
            "runtime_navigation_paths": "Nicht ausgeführt"}


def main():
    root = Path(os.environ.get("ASKIR_V77_PROJECT", str(Path(__file__).resolve().parents[1])))
    report_path = Path(os.environ.get("ASKIR_V77_MAP_REPORT", str(root / "Reports/V77/dornfeld_editor.json")))
    report_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        import unreal
        if os.environ.get("ASKIR_V77_VALIDATE_ONLY") == "1":
            version = unreal.SystemLibrary.get_engine_version()
            if not version.startswith("5.6."):
                raise RuntimeError("UE5.6 required; editor reported " + version)
            for name in REQUIRED_CLASSES:
                if getattr(unreal, name, None) is None:
                    raise RuntimeError("Missing native class: " + name)
            if not unreal.EditorLevelLibrary.load_level(MAP_PATH):
                raise RuntimeError("Existing V77 map cannot load")
            validation = validate_saved_level(unreal, unreal.get_editor_subsystem(unreal.EditorActorSubsystem), root)
            startup_config = configure_startup_maps(root, validation)
            result = {"status": "Bestanden", "coverage": "existing-map load and structural validation",
                      "engine_version": version, "map_package": MAP_PATH, "validation": validation,
                      "startup_map_config": startup_config,
                      "gameplay_status": "Nicht ausgeführt"}
        else:
            result = generate(unreal, root)
    except Exception as exc:
        result = {"status": "Fehlgeschlagen", "map_package": MAP_PATH, "error": str(exc),
                  "traceback": traceback.format_exc(), "gameplay_status": "Nicht ausgeführt"}
        report_path.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
        raise
    report_path.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
    unreal.log("ASKIR_V77_MAP_VALIDATED " + MAP_PATH)


if __name__ == "__main__":
    main()
