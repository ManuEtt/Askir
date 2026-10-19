# ASKIR RPG · V80 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Quellprojekt und echte Original-Assetquellen. Unreal Engine 5.6 und ein Windows-Buildhost fehlen hier weiterhin. C++-/UHT-Kompilierung, Kartenproduktion und Spielbarkeit sind nicht nachgewiesen. Das Archiv enthält keine `.umap`, `.uasset` oder Spiel-EXE.**

V80 erweitert den Quellcode um Torbens kanonische Nebenquest **„Die verlorene Lieferung“ (SQ_001)**: Entscheidung im Dialog, zwei eindeutig zugeordnete Banditen, Sicherung der Fracht und Rückgabe gegen einmalig 100 XP, 16 Gold und den kanonischen Gegenstand `item_002`. Die Entscheidung verändert einmalig die Beziehung zu Waldhütern oder Freien Händlern. Questbelege, Entscheidungen und Fraktionswerte verwenden die vorhandenen persistenten Speicherdaten.

Die neue Editorpipeline verbindet Dornfeld und das Dornfelder Umland. Sie erhält die Brunnenmission sowie Händler und Werkbank aus V79 und ergänzt Torben, die Bauernstraße, das Banditenlager und Hin-/Rückreise per E. Reisesnapshots bleiben bis zur bestätigten Ankunft erhalten. Fehlgeschlagene Ankünfte bieten Wiederholung oder die Rückkehr zum gesicherten Ort; noch ausstehende Reisen blockieren Eingaben, Physik, Schaden und konkurrierende Transaktionen.

Zwei originale 3D-Requisiten wurden tatsächlich mit Blender erzeugt, exportiert und gerendert: Handwagen und Warenkiste. BLEND, GLB, OBJ, MTL, Vorschauen und CC0-Nachweis sind enthalten. Die binären Geometrien wurden geprüft. Ihre Unreal-Importschnittstelle ist integriert, der Engine-Import wurde noch nicht ausgeführt.

Die erhaltenen V77–V79-Systeme umfassen die Dorfmission „Das Flüstern im Brunnen“, Magie mit 24 kanonischen Zaubern, Inventar/Ausrüstung, native UMG-Menüs, Handel/Crafting und Saveformat 5 mit zwei Sicherungsbänken. Originale Rigs und zwölf Clips pro Figur sowie neun WAV-Dateien sind vorhanden. Diese Unreal-Integrationen bleiben unkompiliert.

Offline prüfen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v80.py
```

Der Prüfer führt Python-Regressionen, direkt verwendete portable C++-Produktionsregeln mit g++/UBSan sowie binäre Assetprüfungen aus. `Reports/V80_verification.json` enthält Befehle, tatsächliche Ausgaben und Quelldateihashes. Er trennt erfolgreiche Offline-Prüfungen von blockierten Unreal-/Windows-Tests. ZIP-Erstellung verlangt unveränderte geprüfte Quellen und verifiziert sämtliche Einträge.

Windows mit UE 5.6, Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV80.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V80"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Die Pipeline sichert vor Änderungen, kompiliert den Editor, importiert Assets, erzeugt und validiert beide gespeicherten Karten, führt native Automation aus, erstellt das Win64-Paket und prüft seine Starts. Sie ändert den Startlevel erst nach erfolgreicher Validierung beider Karten. Ein Starttest ersetzt kein manuelles Durchspielen.

Details: [V80-Änderungen und Grenzen](Docs/V80_CHANGES.md), [Weltpipeline](Docs/V80_WORLD_PIPELINE.md), [Originalmodelle](Docs/V80_DELIVERY_PROP_ASSETS.md), [Teststatus](Docs/V80_VERIFICATION.md). Die vollständige Kampagne, 30 abgenommene Levels und ein getesteter Windows-Build bleiben offen.
