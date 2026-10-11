# Askir V43 – Levelportale und Kartenpfade

## Tatsächliche Änderungen
- `AAskirWorldPortal` sucht Zielkarten jetzt unter `/Game/Askir/WorldV42/` statt unter dem nicht passenden `/Game/Maps/`.
- Der Aufruf von `OpenLevel` verwendet den vollständigen Kartenpaketpfad.
- `Tools/setup_world_portals_v43.py` kann in Unreal Editor die C++-Portal-Actors in allen 30 V42-Karten platzieren. `DRY_RUN=True` verhindert zunächst Änderungen. Bereits beschriftete Portale werden nicht dupliziert.
- `Config/WorldMapsToCook_V43.ini.txt` listet die 30 Karten für Packaging.

## Anleitung
1. Unter Windows das Projekt mit UE5.6 kompilieren; Backup erstellen.
2. Mit `Tools/generate_30_individual_levels_v42.py` die 30 Blockout-Maps erzeugen (`DRY_RUN=False`).
3. `Tools/setup_world_portals_v43.py` zuerst als Vorschau, danach mit `DRY_RUN=False` im Unreal Python Editor ausführen.
4. Cook-Liste aus `Config/WorldMapsToCook_V43.ini.txt` übernehmen.
5. Portalüberlappungen, Rückreise, Spawnpositionen, Speichern und Laden im Editor testen.

## Nicht abgeschlossen
Es wurden hier keine `.umap`-Dateien erstellt. Ohne Unreal Engine kann weder kompiliert noch getestet werden. Karten sind Blockouts und keine finalen Spiellevels; Navigation, Beleuchtung, echte Gegner/NPCs, Questketten und finale Assets fehlen.
