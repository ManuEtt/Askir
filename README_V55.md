# Askir RPG V55 — Combat / animation integration preflight

Cumulative UE 5.6 source package based on V54. Fixes a concrete defect in V54: all 90 `enemy_class_path` values used `/Game/.../BP_Name_C` instead of the Unreal generated-class object path `/Game/.../BP_Name.BP_Name_C`. `Tools/bind_encounter_blueprints_v54.py` now receives corrected input.

New six-pawn combat profiles: `Production/Blueprints/combat_animation_profiles_v55.json`. `Tools/configure_combat_profiles_v55.py` applies editable pawn defaults in UE Editor after native compile and Blueprint generation; DRY_RUN=True by default. Verify persistence of generated Blueprint defaults after restart/recompile. No animation montages, Behavior Trees, sound, VFX or real quest IDs are bundled.

Offline validation: `python Tools/validate_combat_pipeline_v55.py` and `python -m unittest discover -s Tests -p 'test_combat_pipeline_v55.py' -v`.

**UE workflow:** compile C++ -> generate V51 Blueprints -> configure V53 AI -> create V42 maps and V49 spawners -> preview V55 combat config -> apply and inspect Blueprint defaults -> preview/apply V54 encounter bindings -> Play In Editor. Back up before disabling dry-run.

**Status:** offline validation only; Unreal Engine is unavailable here. This is not a verified playable build.
