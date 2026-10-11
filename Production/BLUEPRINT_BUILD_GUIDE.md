# Blueprint and native Unreal asset assembly

1. Create `/Game/Askir/Blueprints`, `/Game/Askir/UI`, `/Game/Askir/AI`, `/Game/Askir/Maps`, `/Game/Askir/Data`, `/Game/Askir/Audio`, `/Game/Askir/VFX`, `/Game/Askir/Materials`.
2. Compile C++ then create `BP_AskirCharacter` from `AAskirCharacter`, `BP_AskirPlayerController` from a PlayerController base and `BP_Enemy` from existing enemy character type.
3. Create IA_Move (Axis2D), IA_Look (Axis2D), IA_Jump, IA_Interact, IA_Attack, IA_Block, IA_Dodge; bind in IMC_Default; configure Enhanced Input Local Player subsystem.
4. Import CSV into `DT_quests`, `DT_spells`, `DT_items` using row types in `AskirProductionRows.h`.
5. Create WBP_HUD, WBP_Inventory, WBP_QuestLog, WBP_Dialogue, WBP_Map and WBP_Crafting. Bind widgets to gameplay components; handle null components.
6. Create BB_Enemy (TargetActor, HomeLocation, State), BT_Enemy (patrol/chase/attack), tasks and services in editor.
7. Create BP_QuestGiver, BP_Merchant, BP_EncounterSpawner based on existing native classes.
8. Create five maps from `level_layouts.json`; set player starts, lighting, NavMesh, volumes, enemies, loot, dialogue NPCs and bosses.
9. Create SC_*, SM_* Sound assets, Niagara systems and PBR materials. Import only owned/licensed source assets.
10. PIE test and fix compilation/runtime issues. These native assets cannot be generated or validated without Unreal Editor.
