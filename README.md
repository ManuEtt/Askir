# ASKIR RPG · V83 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Quellprojekt mit Original-Assetquellen. Unreal Engine 5.6 und ein Windows-Buildhost fehlen in dieser Umgebung. Native Kompilierung, Engine-Import und Spielbarkeit sind nicht nachgewiesen. Das Archiv enthält derzeit keine `.umap`, `.uasset` oder Spiel-EXE.**

V83 bindet Dialoggraphen an die native Gesprächsoberfläche an. Torbens Gespräch berücksichtigt den aktuellen Stand der Lieferungsquest und die gespeicherte Hilfs-/Handelsentscheidung. Die bestehenden Questfunktionen führen Annahme und Belohnung aus. Rurik erhält ein eigenes Gespräch über Dornfeld, den Brunnen, den Grauwald und die Krone; gespeicherte Brunnenkunde verändert seine spätere Begrüßung.

Der deterministisch erzeugte C++-Katalog enthält die 24 vorhandenen Dialogvorlagen und Ruriks neuen Dialog. Die 23 noch nicht implementierten Questannahmen bleiben deaktiviert. Antworten tragen stabile IDs und eine Sitzungsrevision. Reichweite, Sicht, Weltzustand und Bedingungen werden beim Auswählen erneut geprüft. Alte Antworten nach Dialogwechsel oder Laden können keine neuen Effekte auslösen. Speicherzugriffe während laufender Gesprächsaktionen sind gesperrt; Abbruch und Actor-Zerstörung während eines Questcallbacks werden berücksichtigt.

Die V83-Editor-Erweiterung platziert Rurik als freundlichen nativen NPC in Dornfeld und prüft nach Speichern/Laden seinen Dialog, vorhandene Guard-Assets, Kollision und Navigation. Die beiden V80-Karten werden anschließend erneut validiert. Diese Editor-Ausführung bleibt hier blockiert. V77–V82 mit Mara-Mission, Lieferung, Weltreise, Wirtschaft, Magie, Speichersystem, Hauptmenü/Einstellungen und Zielerfassung bleiben erhalten.

Offline prüfen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v83.py
```

Der Prüfer führt Python-Regressionen, direkt verwendete portable C++-Produktionsregeln mit g++/UBSan, Kataloggeneratorprüfungen sowie binäre Assetprüfungen aus. `Reports/V83_verification.json` enthält Befehle, tatsächliche Ergebnisse und Quelldateihashes. Die ZIP-Erstellung verlangt unveränderte geprüfte Quellen und prüft sämtliche Archiveinträge.

Windows mit UE 5.6, Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV83.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V83"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Die Pipeline sichert vor Änderungen, kompiliert, importiert Assets, erzeugt/validiert die erhaltenen V80-Karten, ergänzt Rurik, führt native Automation aus und erstellt/prüft das Win64-Paket. Startmarker belegen nur die initialisierte Spielerwelt und ihre Komponenten; tatsächliche Dialogbedienung und Durchspielen benötigen Engine-Abnahme.

Details: [Änderungen](Docs/V83_CHANGES.md), [Gesprächsbackend](Docs/V83_CONVERSATIONS.md), [Dialogdaten](Docs/V83_DIALOGUE_DATA.md), [Gesprächsoberfläche](Docs/V83_CONVERSATION_UI.md), [Pipeline](Docs/V83_PIPELINE.md), [Prüfstatus](Docs/V83_VERIFICATION.md). Die vollständige Kampagne, 30 abgenommene Levels und ein getesteter Windows-Build bleiben offen.
