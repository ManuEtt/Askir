# Askir V51 – Blueprint integration into existing project

V51 includes the complete V50 project and all 24 Blueprint definitions. `Config/DefaultEngine.ini` now points to the Blueprint GameMode. The new Editor script creates missing assets, keeps existing ones, and connects `BP_AskirGameMode` to `BP_AskirPlayer`.

## Steps on Windows with Unreal Engine 5.6
1. Extract ZIP into an empty folder and open `AskirRPG.uproject` with Unreal Engine 5.6 and the Python Editor Script Plugin enabled.
2. Compile the Askir C++ module successfully. Fix compilation errors before proceeding.
3. In `Tools/integrate_blueprints_v51.py`, set `DRY_RUN = False` after a backup.
4. Run the script from the Unreal Editor Python console. Check Output Log for missing native classes and failures.
5. Verify all 24 Blueprint assets under `/Game/Askir/Blueprints`, and check the GameMode's Default Pawn Class.
6. If the Blueprint GameMode was not created, restore the fallback `GlobalDefaultGameMode` value from `Config/DefaultEngine_CppFallback_V51.ini.txt`.
7. Create the 30 maps using the separate existing level scripts; test GameMode, Pawn, NPC and encounters in Play In Editor.

This archive does **not** contain generated `.uasset` files. The script has **not** run in UE here, and no C++ compilation or gameplay test has been performed. Blueprint subclasses alone do not constitute completed behavior graphs, animation assets, widgets, or behavior trees.
