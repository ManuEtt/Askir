# V84 – Editor- und Buildpipeline für SQ_003

`Scripts/Build-AskirV84.ps1` startet `Tools/run_unreal_v84.py`. Der Runner
übernimmt die vollständigen V80–V83-Schritte: Editorbuild, Originalassetimport,
beide vorhandenen Weltverträge, Rurik, native Automation, Win64-Paket und
eigenständige Startprüfungen. Er erzeugt keine zusätzlichen Kartenkopien.

```powershell
.\Scripts\Build-AskirV84.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V84"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Vor einer
Editoränderung wird `Backup/Askir_pre_v84_generation` angelegt; vorhandene
Sicherungen werden nicht überschrieben. `-CheckOnly` prüft nur die Umgebung.
`-UseExistingMap` erhält die V80-Welten und ergänzt fehlende V83/V84-Actors;
widersprüchliche oder doppelte bestehende Identitäten führen zum Abbruch.

Der zusätzliche reale Editorschritt `Tools/build_herb_quest_v84.py`:

- Prüft Katalogfrische direkt im eingebetteten Python sowie kanonische SQ_003-
  Daten: Rurik, ein Erkundungsziel, Rückkehr und 24 Gold/150 XP/`item_004`.
- Importiert und validiert das originale Kräutermodell samt sechs Materialien.
  Die gespeicherten Assets müssen ihre Originalquellen und PBR-Werte behalten.
  `item_004` wird als kanonischer Materialgegenstand unter
  `/Game/Askir/V84/Items/item_004` erzeugt oder überprüft.
- Ergänzt ausschließlich am vorhandenen Rurik den Marker
  `ASKIR_V84_RURIK_GIVER`; seine V83-Identität und Dorfkunde bleiben erhalten.
- Platziert im Umland genau einen nativen `AskirHerbPatchActor` mit Quell-ID
  `SQ003_Meadow_V84`. Root `(1100,900,100)`, visuelle Verschiebung `(0,0,-100)`
  und der maßhaltige Import setzen die Pflanzenfüße auf Bodenniveau.
- Prüft nach Speichern und erneutem Laden beide Welten. Bestehende Actor-
  Objektpfade, Klassen, Labels und Transformationen müssen erhalten bleiben.
  Boden, Spielerkapsel, Zugang `(900,900,100)` und vollständiger Navigationspfad
  werden tatsächlich abgefragt. Die native Querybox muss Sichtstrahlen blocken
  und Spielerbewegung freilassen; Offset `(0,0,-60)`, Ausdehnung `(65,65,45)` und
  der erste reale Sichtstrahltreffer auf den Interaktionsactor werden geprüft.
- Validiert danach erneut den vollständigen V83-Vertrag einschließlich Rurik
  und beider V80-Welten. Ein Erfolgsbericht ohne diese Nachprüfung reicht nicht.

Der Runner lehnt alte Berichte, fehlende reale Pakete, falsche Paketköpfe,
abweichende Materialreferenzen und einen bloßen Editor-Exitcode 0 ab. Ein
vorhandener Paketkopf allein bestätigt keine gültige Unreal-Assetstruktur;
Objekttyp und Inhalte müssen zusätzlich im tatsächlichen Editor geprüft werden.
Ein Start zählt nur mit V80-Spielerwelt sowie V81-Menü, V82-Zielerfassung,
V83-Gespräch und `ASKIR_V84_HERBS_READY` für denselben aktuellen Spieler.
Diese Marker belegen Komponenteninitialisierung, kein Durchspielen der Quest.

Offline lassen sich ausschließlich Quellen und Prüfgrenzen ausführen:

```bash
python Tools/build_herb_quest_v84.py --preflight --report Reports/V84/herb_source_preflight.json
python -m unittest discover -s Tests -p test_herb_pipeline_v84.py -q
```

Die 25 Regressionstests prüfen unter anderem veraltete oder unvollständige
Startnachweise, fremde Spieler, gefälschte Editorberichte, Querygeometrie,
kanonische Materialpakete, Quelldaten, sichere Paketpfade und den Abbruch bei
fehlender Engine. Isolierte Berichtsfixtures ersetzen ausdrücklich keinen
Editorlauf. UE 5.6/Windows, echte Welt-/Asset-Erstellung und die SQ_003-Abnahme
bleiben in dieser Linux-Umgebung **Blockiert** beziehungsweise **Nicht ausgeführt**.
