# V79: Dornfelder Handel und Werkbank

Die neue Pipeline ergänzt den vorhandenen Dorf-Integrationsstand um Edda, eine Werkbank und drei persistente Ressourcen-Pickups. Sie erstellt Unreal-Pakete ausschließlich mit einem tatsächlich laufenden Unreal-Editor 5.6. Im aktuellen Linux-Arbeitsplatz ist diese Ausführung **blockiert**: Unreal Engine 5.6 und ein Windows-Buildhost fehlen. Es wurden hier keine `.umap`- oder `.uasset`-Dateien erzeugt und keine Handels-, Crafting- oder Dorf-Spielsequenz in Unreal ausgeführt.

## Implementierte Daten und Initialisierung

Eine geprüfte gemeinsame Quelle, `Production/Economy/dornfeld_v79.json`, definiert fünf Gegenstände und zwei Startrezepte. `Tools/generate_village_catalog_v79.py` erzeugt daraus native Rezept-, Preis- und Bestandskonstanten. Der Editorbuilder nutzt dieselbe JSON-Datei für echte `UAskirItemData`-Pakete unter `/Game/Askir/V79/Items`.

| Gegenstand | Kaufpreis | Startbestand | Spielparameter |
|---|---:|---:|---|
| Brunnenkraut | 3 | 32 | Zutat, Stapel 99 |
| Gegerbtes Leder | 5 | 20 | Zutat, Stapel 99 |
| Dornfelder Eisenerz | 7 | 30 | Zutat, Stapel 99 |
| Dornfelder Heiltrank | 14 | 6 | `HealAmount=30`, Stapel 20 |
| Dornfelder Jagdschwert | 72 | 2 | `Damage=24`, Reichweite 160, Abklingzeit 0,65 s |

Edda startet mit 500 Gold und kauft zum Faktor 0,5 zurück. Ihr persistenter, innerhalb der Karte eindeutiger Schlüssel lautet `DORNFELD_HAENDLER_EDDA`. Der tatsächlich berechnete Verkaufspreis folgt den nativen Preisregeln; bei Brunnenkraut wird 1 Gold ausgezahlt.

Beide Rezepte sind bei Handwerkstufe 1 bekannt. Zwei Bündel Brunnenkraut ergeben einen Heiltrank; drei Einheiten Eisenerz und zwei Leder ergeben ein Jagdschwert. Ressourcen-Pickups nahe dem Startpunkt liefern genau diese Zutaten. Ihre IDs sind `Dornfeld_V79_Kraut`, `Dornfeld_V79_Leder` und `Dornfeld_V79_Eisenerz`. Sie gehören zum bestehenden persistenten Lootsystem und sind ausdrücklich keine Brunnenscherben für Maras Mission.

`AskirVillageEconomyCatalog::ConfigurePlayer` installiert Catalog und Rezepte erst, wenn alle fünf tatsächlichen Itempakete vorhanden sind. Bereits konfigurierte Designer-Kataloge und Rezepte bleiben erhalten. `ConfigureMerchant` füllt ausschließlich einen ausdrücklich mit Eddas ID identifizierten Händler mit leerem Bestand. Fehlende Pakete werden vor dem Laden geprüft; bei fehlenden Dateien wird nichts teilweise installiert.

## Editor- und Buildpipeline

`Tools/build_dornfeld_v79.py` verwendet die Dorf- und Animationsimporte aus V77/V78. Es erzeugt die neue Zielkarte `/Game/Askir/Maps/Dornfeld_V79`, bewahrt vorhandene Karten und platziert zusätzliche Handels-, Werkbank- und Ressourcen-Akteure. Edda steht bei `(540, -900, 90)`, die Werkbank bei `(-520, -350, 45)`.

Die Pipeline verschiebt jede Startup-Konfigurationsänderung bis nach folgenden Editorprüfungen:

1. Die tatsächliche Karte wurde gespeichert und erneut geladen.
2. Alle erforderlichen Missionsquellen, Truhe und Relaisreferenzen sind erhalten.
3. Die neuen Itempakete existieren physisch und laden als `UAskirItemData` mit den vereinbarten Werten.
4. Händler-ID, Preise, Bestände und Ressourcenreferenzen überstehen das Neuladen.
5. Händler und Werkbank besitzen sichtbare Meshes und blockierende Kollision.
6. Navigation wurde nach den zusätzlichen Kollisionen neu aufgebaut; vollständige Pfade führen vom Spawn zu den Missions- und Wirtschaftsakteuren.

Erst danach setzt der Builder `GameDefaultMap` und `EditorStartupMap` auf Dornfeld V79. Die vorhandenen Produktionsmaterialien und Animationen werden weiterverwendet. Händler und Werkbank verwenden derzeit Engine-Primitive; sie sind konsistente Blockout-Objekte und keine fertigen Produktionsmodelle.

Der Runner `Tools/run_unreal_v79.py` sichert vor Editoränderungen, baut `AskirEditor`, führt den Builder und alle registrierten nativen Askir-Automationstests aus und prüft Berichte und tatsächliche Dateien. Anschließend verlangt er ein echtes Win64-EXE und einen eindeutigen Runtime-Startmarker für Dornfeld V79. Ein erfolgreicher Kopfstart ersetzt keine manuelle Gameplay-Abnahme.

Auf einem Windows-Rechner mit Unreal Engine 5.6, Visual Studio C++-Tools, Windows SDK und Python 3.10+:

```powershell
python -m pip install -r requirements-dev.txt
python Tools/generate_village_catalog_v79.py --check
python Tools/verify_v79.py
powershell -ExecutionPolicy Bypass -File Scripts/Build-AskirV79.ps1 `
  -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' `
  -OutputDirectory 'C:\AskirRuns\V79_001'
```

Das Ausgabeverzeichnis muss neu sein und außerhalb des Projektordners liegen. Falls die echte Karte schon existiert, wird sie erhalten; mit `-UseExistingMap` kann der Runner die bestehende V79-Karte validieren. Der Builder setzt die Konfiguration nur nach erfolgreicher Validierung um.

Offline lässt sich der Eingabevertrag ohne Unreal prüfen:

```bash
python Tools/build_dornfeld_v79.py --preflight --report Reports/V79_economy_source_preflight.json
python -m unittest Tests.test_village_pipeline_v79 -v
```

## Tatsächlich ausgeführte Prüfungen

Am 13.10.2026 wurden 18 neue Python-Tests bestanden. Sie prüfen ungültige Werte, doppelte persistente IDs, fehlende Rezeptreferenzen, Questgegenstände, wirkungslose Verbrauchsgegenstände, Catalog-Synchronität, geometrische Navigationsannäherungen, Startup-Voraussetzungen, exakte Startmarker und die korrekte Blockademeldung bei fehlendem Unreal. `Tools/validate_release.py` und der generierte Catalog-Abgleich wurden ebenfalls bestanden.

`Reports/V79_economy_source_preflight.json` enthält ausschließlich den erfolgreichen Eingabe-/Schemaabgleich. `Reports/V79/dornfeld_editor.json` und `Reports/V79_pipeline_environment.json` dokumentieren die konkrete Engine-/Windows-Blockade. Alle Engine-Komplettprüfungen bleiben blockiert beziehungsweise nicht ausgeführt. Material-, Kollisions-, Navmesh-, Gameplay-, Transaktions-, Speichern/Laden- und visuelle Abnahme sind erst nach tatsächlicher UE-Ausführung nachweisbar.
