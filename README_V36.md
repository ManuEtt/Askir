# Askir RPG V36 – Storywelt-Blockout-Pipeline

Ausgangspunkt: V35. Neue Dateien: `Production/Levels/world_manifest_v36.json` (30 Orte, 5 Kapitel, 3 Kontinente, Nachbarschaften und Story-Anker), `Tools/generate_30_world_blockouts_v36.py` (Unreal-Editor-Python-Generator), `Tests/test_world_manifest_v36.py`, `RELEASE_V36_50_SCHRITTE.md` und `V36_EDITOR_WORKFLOW.md`.

**Wichtig:** Es wurden keine fertigen Unreal-Levels erzeugt. Der Generator erstellt bei Ausführung im Unreal Editor nur begehbare Blockout-Grundflächen mit Platzhalter-Geometrie, PlayerStart und nicht-funktionalen Ausgangsmarkern. NPCs, Quests, NavMesh, funktionierende Übergänge, visuelle Assets und Spieltests fehlen noch. Unreal-Editor-Kompilierung wurde nicht ausgeführt.
