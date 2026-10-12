# ASKIR RPG V74 — Dornfeld Unreal Editor integration playground

Adds `Tools/build_dornfeld_playground_v74.py`, a non-destructive, opt-in Unreal Engine 5.6 Editor Python builder for a **new** prototype map. It places basic collision geometry, a PlayerStart, and instances of existing AskirNPC, AskirQuestTrigger and AskirLootChest C++ actor classes. It assigns initial NPC hostility and unique chest ID. Default is PREVIEW_ONLY=True. It refuses to overwrite existing maps.

**Important:** This is an integration scaffold, not a complete playable mission. Quest IDs and objective routing must be reconciled with actual runtime mission data; Mara dialogue, shard interaction, navmesh, chest inventory, level lighting, full persistence and gameplay remain to be completed and tested in UE 5.6. The Unreal Editor script was not run in this environment. The new tests check script structure and safety, not editor compatibility or gameplay.

Run from Unreal Editor Python console after compiling the Askir module. Back up the project and change PREVIEW_ONLY to False for map creation. Check Output Log for errors. The map will be saved at `/Game/Askir/Prototype/V74_DornfeldPlayground` if the editor API calls succeed.
