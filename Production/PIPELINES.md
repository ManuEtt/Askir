# Askir UE 5.6 Pipelines

## Build
Windows: Install Unreal Engine 5.6, Visual Studio 2022 with Game Development with C++, Windows SDK. Right-click `AskirRPG.uproject` -> Generate Visual Studio project files. Build `AskirEditor Win64 Development` using Visual Studio or UnrealBuildTool. Open editor and run PIE. Packaging via Platforms > Windows > Package Project. No build was executed in this environment.

## Assets
ArtSource FBX/OBJ/GLB must be imported in Unreal Editor into `/Game/Askir/`. Set centimeters, skeleton assignments, material instances, collisions, LODs. GLB import may require Interchange plugin. The manifest is a production requirement, NOT actual finished art.

## Dialogue/Quest
JSON is authoritative design content; `DT_*.csv` can be imported as UE DataTables with matching `FAskirProduction*Row`. Branching dialogue and multi-objective quests need importer/editor Blueprints or further runtime implementation. Do not interpret JSON alone as playable.

## Audio
Import 48kHz WAV, configure SoundClass and SoundMix assets in editor; Music stems are listed but not generated.

## Shader/VFX
Build PBR master materials, instances and Niagara emitters in editor; no `.uasset` generated outside editor.

## Levels
Use `Production/Levels/level_layouts.json` as coordinates and placement instructions; build Landscape, NavMesh, World Partition where specified, place actors and save `.umap` in editor.

## AI
Existing C++ AI controllers remain; create Blackboard/BehaviorTree assets in editor and bind MoveTo, Perception, attack tasks. Native BT assets cannot be substituted by JSON.

## Input/UI/Save
Existing C++ Character and SaveGame classes remain. Create IA_*, IMC_Default, WBP_HUD, WBP_Inventory, WBP_QuestLog and wire bindings in editor. Existing save data schema should be migrated carefully and versioned.
