"""Run in Unreal Editor after native C++ build and V51 Blueprint generation.
DRY_RUN prevents all writes; rerun with False after backup and inspection.
"""
import unreal
DRY_RUN = True
CONTROLLER_PATH = "/Game/Askir/Blueprints/AI/BP_AI_AskirBossPhases"
PAWN_PATH = "/Game/Askir/Blueprints/Enemies/BP_AskirBoss"
NATIVE_PATH = "/Script/Askir.AskirBossAIController"

def run():
    native = unreal.load_class(None, NATIVE_PATH)
    if not native:
        raise RuntimeError("Compile AskirBossAIController C++ before running")
    if not unreal.EditorAssetLibrary.does_asset_exist(PAWN_PATH):
        raise RuntimeError("Generate BP_AskirBoss using V51 first")
    if DRY_RUN:
        unreal.log("[Askir V56] Preview: create/verify %s and assign to %s" % (CONTROLLER_PATH, PAWN_PATH))
        return
    lib = unreal.EditorAssetLibrary
    controller = lib.load_asset(CONTROLLER_PATH)
    if not controller:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", native)
        controller = unreal.AssetToolsHelpers.get_asset_tools().create_asset("BP_AI_AskirBossPhases", "/Game/Askir/Blueprints/AI", unreal.Blueprint, factory)
    if not controller:
        raise RuntimeError("Could not create boss controller Blueprint")
    # Use a distinct BP name: V53 already creates BP_AI_AskirBoss with
    # AAskirEnemyAIController as parent. Reusing it silently loses boss phases.
    if not controller.generated_class().is_child_of(native):
        raise RuntimeError("Boss controller has wrong native parent; do not overwrite")
    controller_class = unreal.load_class(None, CONTROLLER_PATH + ".BP_AI_AskirBossPhases_C")
    pawn_blueprint = lib.load_asset(PAWN_PATH)
    if not controller_class or not pawn_blueprint:
        raise RuntimeError("Missing generated classes")
    pawn_class = unreal.load_class(None, PAWN_PATH + ".BP_AskirBoss_C")
    if not pawn_class:
        raise RuntimeError("Missing generated boss pawn class")
    defaults = unreal.get_default_object(pawn_class)
    defaults.set_editor_property("ai_controller_class", controller_class)
    defaults.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.PLACED_IN_WORLD_OR_SPAWNED)
    lib.save_loaded_asset(controller)
    lib.save_loaded_asset(pawn_blueprint)
    unreal.log("[Askir V56] Saved; reopen Blueprint and verify AI controller class persists")

if __name__ == "__main__":
    run()
