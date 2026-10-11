"""Askir V51: Create V50 blueprints, wire default game mode and pawn in UE 5.6.
Run in Unreal Editor Python console after compiling Askir C++ module.
DRY_RUN=True by default. Back up Content and Config before applying.
"""
import json
from pathlib import Path
import unreal

DRY_RUN = True
ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "Production/Blueprints/blueprint_manifest_v50.json"
CONFIG = ROOT / "Config/DefaultEngine.ini"

def run():
    items = json.loads(MANIFEST.read_text(encoding="utf-8"))["blueprints"]
    lib = unreal.EditorAssetLibrary
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    report = {"preview": [], "created": [], "existing": [], "missing_native_class": [], "failed": [], "configured": []}
    for entry in items:
        path = entry["asset_path"]
        native = unreal.load_class(None, entry["native_path"])
        if native is None:
            report["missing_native_class"].append(entry["native_path"])
            continue
        if lib.does_asset_exist(path):
            report["existing"].append(path)
            continue
        if DRY_RUN:
            report["preview"].append(path)
            continue
        try:
            folder = path.rsplit("/", 1)[0]
            if not lib.does_directory_exist(folder):
                lib.make_directory(folder)
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", native)
            asset = tools.create_asset(entry["name"], folder, unreal.Blueprint, factory)
            if asset and lib.save_loaded_asset(asset):
                report["created"].append(path)
            else:
                report["failed"].append(path + ": create/save failed")
        except Exception as exc:
            report["failed"].append(path + ": " + str(exc))
    mode_path = "/Game/Askir/Blueprints/Core/BP_AskirGameMode"
    pawn_path = "/Game/Askir/Blueprints/Core/BP_AskirPlayer"
    if not DRY_RUN and lib.does_asset_exist(mode_path) and lib.does_asset_exist(pawn_path):
        try:
            mode_bp = lib.load_asset(mode_path)
            pawn_bp = lib.load_asset(pawn_path)
            pawn_class = pawn_bp.generated_class()
            mode_cdo = unreal.get_default_object(mode_bp.generated_class())
            mode_cdo.set_editor_property("default_pawn_class", pawn_class)
            if lib.save_loaded_asset(mode_bp):
                report["configured"].append("GameMode.default_pawn_class -> BP_AskirPlayer")
            else:
                report["failed"].append("Could not save GameMode default pawn")
        except Exception as exc:
            report["failed"].append("GameMode pawn wiring: " + str(exc))
    elif DRY_RUN:
        report["preview"].append("GameMode.default_pawn_class -> BP_AskirPlayer")
    unreal.log("[Askir V51] " + json.dumps(report, ensure_ascii=False))
    return report

if __name__ == "__main__":
    run()
