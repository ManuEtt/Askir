"""V53: create individual AI controller Blueprints and configure enemy pawn defaults.
Execute inside Unreal Editor after compiling C++ and running V51. DRY_RUN by default.
Existing controller Blueprints are NOT overwritten; their CDO values are updated only
when APPLY_EXISTING_PRESETS is explicitly enabled. Back up Content first.
"""
import json
from pathlib import Path
import unreal

DRY_RUN = True
APPLY_EXISTING_PRESETS = False
ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "Production/Blueprints/ai_profiles_v53.json"
BASE_CONTROLLER = "/Script/Askir.AskirEnemyAIController"
PROPERTIES = ("sight_range", "attack_distance", "attack_interval", "hearing_range",
              "memory_seconds", "max_chase_from_home", "retreat_health_fraction",
              "decision_interval", "hostile_to_player")

def run():
    lib = unreal.EditorAssetLibrary
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    profiles = json.loads(DATA.read_text(encoding="utf-8"))["profiles"]
    native = unreal.load_class(None, BASE_CONTROLLER)
    report = {"preview": [], "created": [], "configured": [], "existing_skipped": [], "missing": [], "errors": []}
    if not native:
        report["errors"].append("Native AI controller not compiled: " + BASE_CONTROLLER)
        return report
    for p in profiles:
        name, path = p["controller_name"], p["controller_path"]
        if not lib.does_asset_exist(p["pawn_path"]):
            report["missing"].append(p["pawn_path"])
            continue
        exists = lib.does_asset_exist(path)
        if DRY_RUN:
            report["preview"].append({"pawn": p["pawn"], "controller": name, "existing": exists})
            continue
        try:
            if exists and not APPLY_EXISTING_PRESETS:
                report["existing_skipped"].append(path)
                continue
            if not exists:
                folder = path.rsplit("/", 1)[0]
                if not lib.does_directory_exist(folder):
                    lib.make_directory(folder)
                factory = unreal.BlueprintFactory()
                factory.set_editor_property("parent_class", native)
                bp = tools.create_asset(name, folder, unreal.Blueprint, factory)
                if bp is None:
                    raise RuntimeError("create_asset returned None")
                report["created"].append(path)
            else:
                bp = lib.load_asset(path)
            controller_class = bp.generated_class()
            if controller_class is None:
                raise RuntimeError("Blueprint has no generated class")
            cdo = unreal.get_default_object(controller_class)
            for prop in PROPERTIES:
                cdo.set_editor_property(prop, p[prop])
            pawn_bp = lib.load_asset(p["pawn_path"])
            pawn_class = pawn_bp.generated_class()
            if pawn_class is None:
                raise RuntimeError("Pawn has no generated class")
            pawn_cdo = unreal.get_default_object(pawn_class)
            pawn_cdo.set_editor_property("ai_controller_class", controller_class)
            pawn_cdo.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.PLACED_IN_WORLD_OR_SPAWNED)
            if not lib.save_loaded_asset(bp) or not lib.save_loaded_asset(pawn_bp):
                raise RuntimeError("Failed saving Blueprint")
            report["configured"].append(p["pawn"])
        except Exception as exc:
            report["errors"].append(name + ": " + str(exc))
    unreal.log("[Askir V53] " + json.dumps(report, ensure_ascii=False))
    return report

if __name__ == "__main__":
    run()
