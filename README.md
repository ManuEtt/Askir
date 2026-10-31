# ASKIR RPG · V84 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Quellprojekt mit Original-Assetquellen. UE 5.6 und ein Windows-Buildhost fehlen hier. Native Kompilierung, Engine-Import und Spielbarkeit sind nicht nachgewiesen. Das Archiv enthält derzeit keine `.umap`, `.uasset` oder Spiel-EXE.**

V84 implementiert die vorhandene Nebenquest **Heilkräuter für Mara (SQ_003)**: Rurik bietet Hilfe oder fairen Handel an, der Spieler untersucht das Heilkräutervorkommen im Dornfelder Umland und berichtet Rurik. Die bestehenden Questtransaktionen zahlen genau 24 Gold, 150 XP und einmal `item_004`. Die kanonische Erkundungsaufgabe wird nicht in eine beliebige Sammelmenge umgeschrieben. Inventarplatz, gespeicherte Wahl und einmalige Belohnung werden geprüft.

Ruriks bisherige Dorfkunde bleibt erhalten. Flüchtige Dialogbedingungen werden aus dem Questledger abgeleitet; sie schaffen keinen zweiten gespeicherten Missionszustand. Der neue Questfortschritt erscheint im HUD und Journal. Spielstände behalten Format 5. Zusätzliche Validierung verwirft widersprüchliche Erkundungs-, Entscheidungs- und Belohnungsdaten.

Ein originales CC0-Kräutermodell liegt als echte Blender-, FBX-, GLB- und OBJ-Quelldateien vor. Die Editor-Erweiterung importiert das Modell und den kanonischen Belohnungsgegenstand, markiert Rurik und platziert das Erkundungsobjekt. Speichern/Laden, Erhalt bestehender Actors, echte Sichtlinientreffer, Boden, Kollision und Navigation werden im vorgesehenen Engine-Lauf geprüft. Diese Ausführung ist hier blockiert. Die bisherigen Mara-/Torben-Missionen, Dorfkunde, Magie, Wirtschaft, Weltreise, Menüs und Zielerfassung bleiben erhalten.

Offline prüfen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v84.py
```

`Reports/V84_verification.json` enthält tatsächliche Befehle, Ergebnisse und Quelldateihashes. Die Prüfung führt Python-Regressionen, direkt verwendete portable C++-Regeln mit g++/UBSan sowie binäre Assetprüfungen aus. Die ZIP-Erstellung verlangt unveränderte geprüfte Quellen und prüft alle Archiveinträge.

Windows mit UE 5.6, Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV84.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V84"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Die Pipeline sichert vor Änderungen, kompiliert, importiert Assets, erzeugt/validiert die erhaltenen V80-Karten, ergänzt Rurik und die Kräuterquest, führt native Automation aus und erstellt/prüft das Win64-Paket. Startmarker belegen die initialisierte Spielerwelt und ihre Komponenten; tatsächliche Bedienung und Durchspielen benötigen Engine-Abnahme.

Details: [Änderungen](Docs/V84_CHANGES.md), [Mission](Docs/V84_HERB_MISSION.md), [Dialog](Docs/V84_HERB_DIALOGUE.md), [Speicherung](Docs/V84_HERB_SAVE.md), [Assets](Docs/V84_HERB_ASSETS.md), [Pipeline](Docs/V84_PIPELINE.md), [Prüfstatus](Docs/V84_VERIFICATION.md), [Fortsetzung](Docs/NEXT_SESSION.md).

Die vollständige Kampagne, 30 abgenommene Levels und ein getesteter Windows-Build bleiben offen. 22 weitere deklarierte Nebenqueststarts sind weiterhin deaktiviert und zählen nicht als spielbare Quests.
