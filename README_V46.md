# Askir RPG V46 – Gameplay-Anker für 30 Karten

- `Production/Levels/world_gameplay_anchors_v46.json`: 240 strukturierte Einträge für 30 Regionen (150 POIs und 90 Encounter-Positionen).
- `Tools/validate_world_gameplay_v46.py`: Offline-Prüfung auf eindeutige IDs, Koordinaten, Kartenzuordnung und Vollständigkeit.
- `Tools/place_gameplay_anchors_v46.py`: Unreal-Editor-Skript, das nach Erzeugung der Karten sichtbare TargetPoint-Anker setzt. Standardmäßig `DRY_RUN=True`.
- `Tests/test_world_v46.py`: drei Offline-Tests einschließlich fehlerhafter Eingabedaten.

## Reihenfolge
1. Unreal Engine 5.6 und Python Editor Script Plugin aktivieren, Projekt kompilieren.
2. `Tools/generate_30_individual_levels_v42.py` im Editor ausführen (`DRY_RUN=False` nach Projektbackup).
3. `Tools/place_gameplay_anchors_v46.py` zunächst als Vorschau, danach `DRY_RUN=False`.
4. Echte NPCs, Quests, Gegner-Blueprints, NavMesh und Assets manuell/mit Folgeskripten integrieren und testen.

**Wichtig:** Keine `.umap`-Dateien oder fertig spielbaren Level enthalten. Unreal wurde hier nicht ausgeführt. Vorhandene V42-Level werden nicht überschrieben. Die 50 Themen sind kein Nachweis von 50 implementierten Gameplay-Features.
