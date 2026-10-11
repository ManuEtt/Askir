# Askir RPG V47 – Weltprüfung und Editor-Abnahme

## Neue Werkzeuge
- `Tools/audit_world_v47.py`: vergleicht drei Welt-Datenquellen, prüft Kartenidentität, Missionen, Portale, Koordinaten, 240 Gameplay-Anker und vollständige Erreichbarkeit. Standard-Python; ohne Unreal lauffähig.
- `Tools/inspect_unreal_world_v47.py`: ausschließlich lesende Prüfung der tatsächlich im Unreal Editor existierenden 30 Karten. Prüft je Karte Ladefähigkeit, V46-Anker, V44-Encounter-Zonen, Portallabel und PlayerStarts. **Nicht in dieser Umgebung ausgeführt.**
- `Tests/test_world_v47.py`: vier Tests mit mutierten fehlerhaften Weltdaten.
- `Production/Levels/world_audit_v47.json`: nachvollziehbarer Offline-Prüfbericht.

## Reihenfolge für die Integration
1. UE5.6 und Python Editor Script Plugin installieren, Askir C++ kompilieren.
2. `python Tools/audit_world_v47.py` lokal ausführen; bei Fehlern nicht mit dem Editor fortfahren.
3. Nach Backup V42-Generator und anschließend Portal-/Gameplay-Skripte mit `DRY_RUN=True` prüfen und gezielt ausführen.
4. `Tools/inspect_unreal_world_v47.py` im Unreal Editor ausführen. Es speichert keine Karten und ändert keine Actors.
5. Fehlende PlayerStarts, NavMesh, echte NPCs, Gegner, Quests, Animationen, Assets und Übergänge manuell/programmatisch ergänzen, anschließend Play-In-Editor und Windows-Build testen.

**Status:** Keine echten `.umap`-Dateien erstellt. Keine UE-Kompilierung, keine Spielbarkeit bestätigt. Die Audit-Ergebnisse belegen nur Konsistenz der Quelldaten.
