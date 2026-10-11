# Askir V42 – 30-Level-Designpaket

Diese Version ergänzt V41 um **30 individualisierte Levelentwürfe** (150 benannte Schauplätze, 90 Encounter-Marker) und einen UE5.6-Editor-Python-Generator für 30 unterschiedliche Blockout-Maps.

## Ausführen
1. Projekt unter Windows in Unreal Engine 5.6 öffnen und C++ kompilieren.
2. Editor Scripting Utilities und Python Editor Script Plugin aktivieren.
3. `Tools/generate_30_individual_levels_v42.py` prüfen; `DRY_RUN=False` setzen.
4. Im Unreal Python Editor `exec(open(r'ABSOLUTER_PFAD_ZUM_SCRIPT', encoding='utf-8').read())` ausführen.
5. 30 Karten unter `/Game/Askir/WorldV42` prüfen. Vorher Backup anlegen; bestehende Karten werden übersprungen.

## Nicht fertig
Dies sind **Blockouts** und Missions-/Encounter-Marker. Es fehlen finale Landschaften, Materialien, Assets, Beleuchtung, NavMesh, funktionale Gegner/NPCs, Storyquests, funktionale Portale, Bossmechaniken, Sound, Cutscenes, QA und Windows-Build. Das Skript wurde hier nicht im Unreal Editor ausgeführt.
