# V36 – 30 Story-Regionen im Unreal Editor erzeugen

1. Vollständiges Projekt sichern und in UE 5.6 öffnen; Python Editor Script Plugin aktivieren.
2. `python Tests/test_world_manifest_v36.py` außerhalb von Unreal ausführen.
3. In `Tools/generate_30_world_blockouts_v36.py` `DRY_RUN=True` belassen und im Unreal-Python-Kontext ausführen: `exec(open(r"C:\Pfad\AskirRPG\Tools\generate_30_world_blockouts_v36.py", encoding="utf-8").read())`.
4. Log auf alle 30 erwarteten Karten prüfen. Danach `DRY_RUN=False` setzen und erneut ausführen. **Bestehende Karten werden nicht überschrieben.**
5. Karten unter `/Game/Askir/WorldBlockoutsV36/` öffnen, Boden und PlayerStart prüfen. Platzhalter für Ausgänge sind noch **keine** Levelwechsel.
6. Für jede Region echte Assets, NavMesh, Beleuchtung, Audio, NPCs, Quest-Trigger und Übergangsaktoren integrieren und in PIE testen.
7. `Production/Levels/world_manifest_v36.json` ist ein Story-/Weltgraph-Vorschlag; kanonische Namen und geografische Zuordnung vor finaler Integration mit der Lore abgleichen.

## Abnahmegrenze

Ein erzeugter Blockout ist **kein fertiger spielbarer Level**. Die 30-Level-Anforderung bleibt offen, bis alle 30 Karten im Editor existieren und einzeln mit Quests, Übergängen, Speichern/Laden und Gameplay erfolgreich getestet sind.
