# Askir RPG V39 – Reisevalidierung und Save-Kompatibilität

Aufbauend auf V38 wurde die Prüfung des Ziel-Map-Pakets **vor** dem Speichern des Reisesnapshots ergänzt. Der SaveGame-Datensatz enthält jetzt eine Versionsnummer und den Quellkartennamen; unbekannte Formatversionen werden beim Laden abgewiesen. Zusätzlich gibt es einen Offline-Validator für das Weltmanifest und Regressionstests für fehlerhafte Verbindungen und doppelte Karten-IDs.

**Wichtig:** Die Map-Pfadannahme `/Game/Maps/<MapName>` muss beim tatsächlichen Unreal-Editor-Export geprüft werden. V39 ist weder in UE 5.6 kompiliert noch im Editor oder als Windows-Build getestet. Keine neuen `.umap`-Dateien enthalten. Die 50 Themen sind in `RELEASE_V39_50_SCHRITTE.md` einzeln ausgewiesen.

Offline-Test: `python -m unittest discover -s Tests -p test_world_v39.py`.
