# ASKIR RPG · V81 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Quellprojekt mit Original-Assetquellen. Unreal Engine 5.6 und ein Windows-Buildhost fehlen in dieser Umgebung. Native Kompilierung, Engine-Import und Spielbarkeit sind nicht nachgewiesen. Das Archiv enthält derzeit keine `.umap`, `.uasset` oder Spiel-EXE.**

V81 ergänzt ein natives UMG-Hauptmenü: neues Spiel, laufende Sitzung fortsetzen, geprüften Spielstand laden, Einstellungen und Beenden. Ein Neustart aus einer Sitzung verlangt eine Bestätigung und hält den bisherigen Snapshot bis zur bestätigten neuen Welt vor. Gespeicherte Partien werden dabei erhalten.

Die Einstellungen umfassen Auflösung und Fenstermodus, Grafikqualität, V-Sync, FPS-Limit, Gesamtlautstärke, Kameraempfindlichkeit und vertikale Invertierung. Unbestätigte Anzeigeänderungen werden nach 15 Echtzeitsekunden zurückgenommen. Konfigurationsänderungen werden erst bei ausdrücklicher Übernahme gespeichert und anschließend von der Datei zurückgelesen. Die native Bedienung und Darstellung bleiben in Unreal zu prüfen.

V80-Inhalte bleiben erhalten: Dornfelds Brunnenmission mit Mara, Torbens Nebenquest „Die verlorene Lieferung“, Hin-/Rückreise ins Umland, Händler und Werkbank. Ebenso erhalten bleiben die 24 kanonischen Zauber, Inventar/Ausrüstung und Saveformat 5 mit zwei Speicherbänken. Die Originalquellen umfassen geriggte Figuren mit Animationen, neun WAV-Dateien sowie Handwagen und Warenkiste. Die Weltpipeline verwendet weiter die bestehenden V80-Kartenpfade; V81 erzeugt keine zusätzlich gezählten Kartenkopien.

Offline prüfen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v81.py
```

Der Prüfer führt Python-Regressionen, portable C++-Produktionsregeln mit g++/UBSan sowie binäre Assetprüfungen aus. `Reports/V81_verification.json` enthält die Befehle, tatsächlichen Ausgaben und geprüften Quelldateihashes. Offline-Erfolg wird von blockierten Unreal-/Windows-Tests getrennt. Die ZIP-Pipeline verlangt unveränderte geprüfte Quellen und prüft alle Archiveinträge.

Windows mit UE 5.6, Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV81.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V81"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Die Pipeline sichert vor Änderungen, kompiliert den Editor, importiert Assets, erzeugt und validiert beide V80-Karten, führt native Automation aus, erstellt das Win64-Paket und prüft dessen Start. Der Starttest verlangt die wiederhergestellte Spielerwelt vor dem Titelmenü-Marker. Er ersetzt keine Sicht-, Eingabe- oder Durchspielprüfung.

Details: [Frontend](Docs/V81_FRONTEND.md), [Einstellungen](Docs/V81_SETTINGS.md), [Änderungen](Docs/V81_CHANGES.md), [Prüfstatus und Engine-Abnahme](Docs/V81_VERIFICATION.md), [V80-Weltpipeline](Docs/V80_WORLD_PIPELINE.md), [Assetquellen](Docs/V80_DELIVERY_PROP_ASSETS.md). Die vollständige Kampagne, 30 abgenommene Levels und ein getesteter Windows-Build bleiben offen.
