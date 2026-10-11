# Askir V45 – Einheitliche 30-Level-Pipeline

## Gelöst
Die bisherigen Editor-Skripte hatten einen echten Integrationsfehler: Der V36-Generator erzeugte Karten unter `/Game/Askir/WorldBlockoutsV36`, während C++-Portale, V43- und V44-Skripte `/Game/Askir/WorldV42` erwarteten. Der Generator ist nun auf `/Game/Askir/WorldV42` umgestellt. Die neue Datei `Config/WorldMapsToCook_V45.ini.txt` listet alle 30 Karten unter dem gleichen Pfad.

## Offline prüfen
`python Tools/preflight_world_v45.py`
`python -m unittest discover -s Tests -p 'test_world_v45.py' -v`

## In Unreal Engine 5.6 (Windows)
1. Projekt sichern, Askir C++ kompilieren.
2. Den Preflight ausführen und die neue Cook-Liste in die Packaging-Einstellungen übernehmen (die Datei `.ini.txt` ist eine Vorlage, keine automatisch geladene UE-Konfiguration).
3. `Tools/generate_30_world_blockouts_v36.py` zunächst mit `DRY_RUN=True` ausführen, danach nur nach Sichtprüfung auf `False` setzen.
4. `Tools/setup_world_portals_v43.py` ebenso zunächst in Vorschau, dann nach Prüfung ausführen.
5. `Tools/integrate_30_level_gameplay_v44.py` ebenfalls separat in Vorschau ausführen; Spawner benötigen einen echten Enemy-Blueprint.
6. Jede Karte auf Kollision, Spawn, Navigation, Reisen und Speichern testen.

## Grenzen
Keine erzeugten `.umap`-Dateien, keine getestete UE5.6-Kompilierung, keine fertigen Gegner/NPCs, keine vollständige spielbare Welt. Die Generatoren sind nicht im Unreal Editor ausgeführt worden. Der neue Runner startet absichtlich keine automatischen Schreiboperationen.
