"""V52: idempotent post-generation configuration for native-backed Askir Blueprints.
Run inside Unreal Editor after V51 generator. Default: read-only preview.
No event graphs, widget assets or behavior trees are generated here.
"""
import json
from pathlib import Path
import unreal
DRY_RUN = True
ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "Production/Blueprints/blueprint_integration_v52.json"
def run():
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    lib = unreal.EditorAssetLibrary
    report = {"preview": [], "configured": [], "missing": [], "errors": []}
    by_name = {b["name"]: b for b in data["blueprints"]}
    controller_entry = by_name.get("BP_EnemyAIController")
    controller = lib.load_asset(controller_entry["asset_path"]) if controller_entry and lib.does_asset_exist(controller_entry["asset_path"]) else None
    controller_class = controller.generated_class() if controller else None
    for preset in data["ai_presets"]:
        name = preset["pawn"]
        entry = by_name[name]
        path = entry["asset_path"]
        if not lib.does_asset_exist(path):
            report["missing"].append(path)
            continue
        if not controller_class:
            report["missing"].append("BP_EnemyAIController")
            continue
        if DRY_RUN:
            report["preview"].append(name + " -> " + preset["controller"])
            continue
        try:
            pawn_bp = lib.load_asset(path)
            pawn_cdo = unreal.get_default_object(pawn_bp.generated_class())
            pawn_cdo.set_editor_property("ai_controller_class", controller_class)
            pawn_cdo.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.PLACED_IN_WORLD_OR_SPAWNED)
            if lib.save_loaded_asset(pawn_bp):
                report["configured"].append(name)
            else:
                report["errors"].append(name + ": save failed")
        except Exception as exc:
            report["errors"].append(name + ": " + str(exc))
    # AI properties are configured on a single controller Blueprint class, not per pawn:
    # per-enemy tuning needs distinct controller subclasses, implemented in a later phase.
    unreal.log("[Askir V52] " + json.dumps(report, ensure_ascii=False))
    return report
if __name__ == "__main__":
    run()
