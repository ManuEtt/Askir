# Askir RPG V29 – Loot & Dorfmission

Basis: V28. Enthält neue C++-Actors `AAskirLootPickup` und `AAskirLootChest`, Missionsdesign, CSV und Testplan. Keine kompilierten Binaries, keine nativen `.umap`/`.uasset`-Assets. Die neue Quest ist als Missionsablauf beschrieben, nicht als vollständig verdrahtete Quest-DataAsset-Instanz.

- `RELEASE_V29_50_SCHRITTE.md` – 50 dokumentierte Aufgaben
- `V29_UNREAL_TESTPLAN.md` – Editor-Testfälle
- `Production/Quests/dornfeld_first_playable_mission.json` – Missionsablauf
- `Production/DataTables/DT_DornfeldLoot_V29.csv` – Beute-Konfiguration

Die Loot-Actors können über Blueprint abgeleitet und mit Meshes versehen werden. Eine persistente Beute-Actor-ID, Save/Load für Truhen und eine UI werden in späteren Versionen benötigt.
