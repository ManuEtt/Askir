# Askir V52 – Blueprint AI configuration

Contains V51 project plus 50 tracked topics (1351–1400). New manifest and safe Unreal Editor post-configuration tool.

1. Compile C++ in UE 5.6.
2. Run V51 integration script after setting DRY_RUN=False.
3. Run `Tools/configure_enemy_blueprints_v52.py` in preview, inspect results, then set DRY_RUN=False and rerun.
4. Run Play-In-Editor and verify AI navigation, possession and combat.

**Limitation:** AI preset values in the manifest are *not* automatically applied individually: all six enemies share one AI controller Blueprint. Distinct subclasses or a per-pawn config component are needed. No UE editor or compiler run here.
