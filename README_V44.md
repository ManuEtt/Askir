# Askir V44 – Gameplay-Integration für 30 Level

## Neu
- `Tools/integrate_30_level_gameplay_v44.py`: platziert Encounter-Zonen in bestehenden Karten; optional Spawner mit explizit konfigurierter Gegner-Blueprint-Klasse. `DRY_RUN=True` standardmäßig. Duplikate werden anhand stabiler Actor-Labels vermieden.
- `Tools/validate_gameplay_v44.py`: Offline-Validierung der 30 Level, 150 Orte, 90 Begegnungen und aller gegenseitigen Kartenverbindungen.
- `RELEASE_V44_50_SCHRITTE.md`: Themen 1051–1100.

## Editorablauf (Windows, UE5.6)
1. Projekt sichern und C++ kompilieren.
2. Mit V42-Generator fehlende Blockout-Maps anlegen; V43-Portale separat platzieren.
3. `integrate_30_level_gameplay_v44.py` im Unreal-Python-Editor zuerst als Vorschau ausführen.
4. Enemy Blueprint in `ENEMY_BLUEPRINT_PATH` eintragen und `DRY_RUN=False` setzen; Skript ausführen.
5. Questtrigger nur nach Abgleich echter Quest-/Objective-IDs aktivieren. Ohne Blueprint entstehen keine Spawner, nur Encounter-Zonen.
6. Alle Maps, Kollisionen, Spawns, Quests, Navigation und Speichern in UE testen.

## Grenzen
Keine `.umap`-Dateien in diesem ZIP. Keine fertigen Landschaften, NPC-Szenen, Quests oder Gegner-Blueprints. Das Skript ist nicht in Unreal getestet. Level sind **nicht vollständig spielbar**.
