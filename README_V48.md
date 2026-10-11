# Askir V48 – Encounter-Konfigurationen und Offline-Zustandsmodell

## Neu
- `Production/Levels/encounter_profiles_v48.json`: 90 datenbasierte Begegnungen für alle 30 Regionen mit Gegner-Archetyp, Anzahl, Aktivierungsradius und Elite-Stufe.
- `Tools/encounter_runtime_v48.py`: kleines, UE-unabhängiges Zustandsmodell (dormant/active/cleared), Quest-Gate, Kill-Zähler, Reload-Reset und Snapshot-Restore.
- `Tools/validate_encounters_v48.py`: Vergleich der 90 Konfigurationen mit den V42-Leveldaten, einschließlich Positionen und IDs.
- `Tests/test_encounters_v48.py`: 20 Offline-Tests.
- `RELEASE_V48_50_SCHRITTE.md`: 30 regionale Datenaufgaben + 20 überprüfbare Mechaniken/Prüffälle.

## Ausführen
```bash
python Tools/validate_encounters_v48.py
python -m unittest discover -s Tests -p 'test_encounters_v48.py' -v
```

## Grenzen
Die Gegner-Archetypen sind Design-IDs, keine vorhandenen Blueprint-Assets. Das Zustandsmodell ist **noch nicht** mit UE C++/Blueprints, Spawn-Actors, Quest-Events oder SaveGame verbunden. Keine neuen `.umap`-Dateien, kein UE5.6-Compile oder PIE-Test. Die 50 Themen bezeichnen konkrete Daten-/Testarbeiten, nicht 50 vollständig spielbare Features.
