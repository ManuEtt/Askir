# Askir V28 – Production specification and source extension

**Scope:** Audited existing V27 files; added production-grade content definitions, 24 sidequest designs, 5 level layouts, 24 spell definitions, 60 item definitions, 16 enemy designs, loot/crafting, branching dialogues, world lore, Unreal C++ reflected DataTable types, build/import/QA guidance.

**Existing:** 54 .cpp, 57 .h, 235 total files before changes; 0 native `.uasset`, 0 `.umap`.

**Retain:** Existing combat, quest, NPC, inventory, progression, save, trade, simulation, art sources; review/extend rather than wholesale replacement.

**Extend:** C++ gameplay hooks to consume imported DataTables, runtime quest/dialogue interpreter, UI, Enhanced Input bindings, AI BT/BB assets, spell casting and level content.

**Replace only after tests:** Any duplicated or conflicting interaction/dialogue components after migration.

**Missing:** Actual UE Blueprints, playable levels, authored animations, complete audio, textures, Niagara assets and UE editor compilation.

**Honest status:** This is a complete **design/content specification plus partial native C++ implementation**, not a complete tested game. Unreal-generated binary assets must be created in Unreal Editor.
