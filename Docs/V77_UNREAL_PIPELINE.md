# V77: echte Editor-Integration und Windows-Pipeline

Status dieser Lieferung: **Unreal-Ausführung blockiert**, solange keine UE5.6-Windows-Installation verfügbar ist. Der Generator ist Quellcode. Eine JSON-Datei, ein lokaler Python-Test oder ein erfolgreicher Syntaxcheck erzeugt keine `.umap` und belegt keine Spielbarkeit.

## Tatsächlich implementierter Ablauf

`Scripts/Build-AskirV77.ps1` ruft `Tools/run_unreal_v77.py` auf. Die Pipeline prüft Engine-Version und Plugins, sichert das Projekt vor Editoränderungen, baut `AskirEditor`, erstellt die Dorfmap über den PythonScript-Commandlet, führt registrierte native Automation-Tests aus, baut und kocht die Dorfsequenz für Win64 und startet den archivierten Prozess. Jeder Schritt benötigt seinen eigenen Nachweis und stoppt nach einem Fehler. Ausgabe und Sicherung müssen außerhalb des Projektordners liegen.

Der Generator `Tools/build_dornfeld_v77.py` schreibt echte Pakete mit Unreal APIs: `/Game/Askir/Maps/Dornfeld_V77`, vier Item-DataAssets und sechs Materialien. Außerdem importiert er die sieben Original-WAV aus `ArtSource/Audio_V77` als echte SoundWaves unter `/Game/Askir/V77/Audio`, passend zu den nativen Cue-Namen. Die Map enthält begehbaren Boden mit Kollision, offene Wege, einfache Häuser, Brunnen, eine überdachte Kellerkammer, PlayerStart, dynamische Beleuchtung, SkyAtmosphere, Nebel, eine AmbientSound-Instanz und echte Navigationsvolumen-Geometrie. `UAskirEditorLevelTools` erzeugt dafür eine CubeBuilder-Brush; ein leeres, lediglich skaliertes NavMeshBoundsVolume wird nicht als funktionierende Geometrie akzeptiert. Die Helfer hängen nur im Editor von UnrealEd ab, Windows-Runtime-Builds behalten sichere Stubs.

Mara ist ein `AskirDornfeldEventActor` mit `ElderDialogue`. Drei echte `AskirLootPickup` übertragen drei unterschiedliche Quest-Items. Zwei `AskirNPC` liefern Todesevents. Eine echte `AskirLootChest` enthält das Brunnensiegel. Sechs `AskirDornfeldMissionRelay` verbinden jeweils genau eine Quelle über stabile IDs. Die native Missionslogik setzt die Reihenfolge, Kampffreigabe und Belohnung durch. Generische QuestID/ObjectiveID bleiben bei diesen Quellen leer, damit der native Journal-Abgleich keine Ziele doppelt zählt. Gegner-Kill-XP ist für diese zwei Missionsgegner null; der Missionsabschluss vergibt die geforderten 180 XP.

Nach dem Speichern muss die echte Datei `Content/Askir/Maps/Dornfeld_V77.umap` vorhanden sein. Der Editor lädt sie erneut. Validiert werden erforderliche Actors, genaue Relay-Referenzen, persistente IDs, Itemreferenzen, GameMode, Navigation-Brush-Ausdehnung und echte Boden-Kollisionstraces für PlayerStart und beide Gegner. Diese strukturelle Prüfung ersetzt keine Spielsitzung.

Erst nach dieser erfolgreichen Prüfung schreibt der Generator `GameDefaultMap` und `EditorStartupMap` in `Config/DefaultEngine.ini` auf die tatsächlich vorhandene Dorfmap um. Bei fehlender `.umap`, fehlgeschlagenem Reload oder fehlgeschlagener Strukturprüfung bleiben die ursprünglichen Startkarten erhalten. Andere INI-Werte und mehrfache Unreal-Arrayeinträge bleiben unverändert.

## Windows starten

Voraussetzungen: UE **5.6.x** für Windows, Python **3.10+**, kompatibles Visual Studio C++/Windows SDK. UBT prüft die tatsächliche Compilerverfügbarkeit. Die aktivierten Editor-Plugins sind `PythonScriptPlugin` und `EditorScriptingUtilities`. Alle Codeänderungen müssen vor der Generierung im Editor-Modul kompiliert sein.

```powershell
.\Scripts\Build-AskirV77.ps1 `
  -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' `
  -OutputDirectory 'D:\AskirBuilds\V77_FirstRun'
```

Nur die Umgebung prüfen:

```powershell
.\Scripts\Build-AskirV77.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' -OutputDirectory 'D:\AskirBuilds\V77_Check' -CheckOnly
```

Die Map wird aus Schutz vor Datenverlust nicht überschrieben. Für eine bereits erfolgreich generierte Map prüft `-UseExistingMap` deren vorhandene Inhalte und Referenzen erneut. Verwende pro Lauf ein neues Ausgabe-Verzeichnis, damit alte Builds/Testergebnisse nicht als neue Nachweise gelten. Nach einem Fehler bleiben Sicherung und Logs erhalten; eine teilweise generierte Map muss im Editor geprüft werden, bevor sie bewusst ersetzt wird.

Rückgabecodes: `0` Pipeline erfolgreich oder Umgebung erfolgreich geprüft, `1` ausgeführter Schritt fehlgeschlagen, `2` technische Umgebung blockiert. `pipeline.json` unterscheidet ausdrücklich **Bestanden**, **Fehlgeschlagen**, **Nicht ausgeführt**, **Blockiert**. Ein Umgebungstest lässt spätere Schritte als „Nicht ausgeführt“ stehen.

## Grenzen der Nachweise

Automation benötigt einen frischen `Automation/index.json` mit tatsächlich ausgeführten, erfolgreichen Tests. Kein Bericht, null Tests, fehlende registrierte Tests, offene Tests oder ein Fehler bedeuten Misserfolg. Der Editor-Rückgabecode allein genügt nicht. Die einzelnen nativen Testfälle bestimmen die Testabdeckung; ein erfolgreicher Bericht belegt keine manuelle End-to-End-Prüfung.

Cook/Package erwartet genau eine reale `Binaries/Win64/Askir.exe`. Der Runner startet diese außerhalb des Editors mit NullRHI **ohne Kartenargument**, erwartet `ASKIR_V77_RUNTIME_READY` für `Dornfeld_V77` im nativen Log und beobachtet den laufenden Prozess weitere fünf Sekunden. Ein Bereitschaftsmarker aus einer anderen Karte ist ein Fehler. Danach beendet er den Smoke-Prozess bewusst. Das prüft Prozess-/Weltinitialisierung und den regulären `GameDefaultMap`-Start. Grafik, Kamera, Eingabegeräte, Kämpfe und vollständiger Missionsablauf müssen separat im sichtbaren Build geprüft werden. Das Feld `game_acceptance` bleibt selbst nach einer erfolgreichen Pipeline „Nicht ausgeführt“.

Die Cook-Liste enthält **Dornfeld**, keine behauptete 30-Level-Kampagne. Die V42/V74-Generatoren erzeugen Blockouts beziehungsweise Vorschauen und wurden nicht als fertige Levels übernommen. Diese Version erstellt keine leeren `.umap` oder umbenannten Textdateien.

Die Umgebungselemente sind bewusst Primitive aus `/Engine/BasicShapes`; Produktionsmodelle, Rigs und Animationssets fehlen. Engine-Inhalte unterliegen der Unreal Engine-Lizenz. Die neu generierten Itemdaten/Materialparameter stammen aus dem Projektcode. Die Original-Sounds sind prozedural erstellt; Details und Lizenz stehen in [V77_AUDIO.md](V77_AUDIO.md). Ob SoundWave-Import und Runtime-Wiedergabe funktionieren, ist erst nach tatsächlicher UE-Ausführung belegt. `LICENSE_NOTICE.md` bleibt für Projekt-Lore und weitere Inhalte maßgeblich. Öffentliche Weitergabe setzt die entsprechenden Rechte voraus.

## Lokale Tests ohne Unreal

```text
python -m unittest discover -s Tests -p test_unreal_pipeline_v77.py -v
python Tools/run_unreal_v77.py --output ../AskirV77_Preflight --check-only
```

Die Python-Tests prüfen insbesondere, dass fehlende Engine, fehlende Maps und unvollständige Testberichte niemals erfolgreich klassifiziert werden. Der zweite Befehl dokumentiert die konkrete Umgebung. Unreal-kompilierte Helfer, API-Interoperabilität, Paketgenerierung, tatsächliche Navigation, Spielbarkeit und Windows-Build bleiben bis zur echten Engine-Ausführung ungetestet.
