# Originale Lieferungsobjekte V80

Die beiden ursprünglichen 3D-Modelle wurden mit Blender 4.3.2 tatsächlich erzeugt,
exportiert und gerendert. Sie sind keine Unreal-Assets. Der Import in UE 5.6 und
Kollisionen im Spiel sind wegen der hier fehlenden Engine **blockiert**.

| Objekt | Geometrie | Abmessungen in cm (X/Y/Z) | Materialien |
| --- | --- | --- | --- |
| SM_AskirDeliveryCart | 2.400 Quellvertices, 4.532 Dreiecke | 298,42 / 173,70 / 128,15 | fünf |
| SM_AskirSupplyCrate | 1.744 Quellvertices, 3.232 Dreiecke | 100,69 / 85,61 / 76,05 | fünf |

Der offene Handkarren besteht aus einzelnen Holzplanken, zwei Rädern mit Speichen,
eisernen Reifen, Achse, Nieten und beschlagenen Ecken sowie zwei langen Griffen.
Die verschlossene Vorratskiste besitzt Planken, Querleisten, diagonale Verstrebungen,
Nägel und zwei Hanfbindungen. Die Materialien verwenden eigene Farben sowie
Rauheits- und Metallwerte. Es wurden keine externen Modelle, Texturen oder Vorlagen
heruntergeladen. Die Originaldateien und gerenderten Vorschauen stehen unter
CC0-1.0; siehe `ArtSource/Delivery_V80/LICENSE_CC0.txt`.

Für jedes Modell liegen echte `.blend`, `.glb`, `.obj`, `.mtl` und eine gerenderte
960×720-PNG-Vorschau vor. `PROVENANCE.json` enthält SHA256 und Größe sämtlicher elf
Dateien sowie Geometriezahlen und Grenzen. Das OBJ verwendet Meter, +X entlang der
Griffe und +Z nach oben; das GLB verwendet die Standardachsen von glTF (+Y nach
oben). Der Ursprung liegt auf dem Boden unter dem Objekt. Der Karren reicht in X
von −110 bis +188,42 cm; beim Platzieren muss besonders die Griffseite frei bleiben.

```sh
blender -b --python Tools/build_delivery_props_v80.py
python Tools/validate_delivery_props_v80.py
python Tools/import_delivery_props_v80.py --preflight
python -m unittest Tests.test_delivery_props_v80 -v
```

Die Geometrieerzeugung enthält keine Zufallszahlen. OBJ und GLB verwenden dieselben
triangulierten Originalmeshes. Blender-Dateien und Renderausgaben sind keine
reproduzierbaren Buildarchive; nach einer Regenerierung wird die Provenienz frisch
berechnet und der Validator muss erneut erfolgreich laufen.

Der vollständige Offline-Validator liest echte binäre GLB-Accessor-/Indexbuffer und
OBJ-Dreiecke. Er prüft endliche Koordinaten und Normalen, gültige Indexbereiche,
nicht entartete Dreiecke, die Gleichheit sämtlicher Dreiecke samt Materialzuordnung
zwischen beiden Formaten, Materialfarben/Rauheit/Metall, Grenzen, Maßeinheiten,
Dateiheader und alle SHA256-Werte. Er berücksichtigt die an harten Kanten und
Materialgrenzen zusätzlichen glTF-Vertices. Die 13 Tests enthalten bewusst
beschädigte Koordinaten, Normalen, Indizes, Dreiecke, Materialien und Provenienz.
Die beiden tatsächlichen Vorschauen wurden visuell geprüft.

`import_delivery_props_v80.py` stellt folgende Python-Schnittstellen bereit:

- `source_preflight(project_root)` prüft Quellen mit der Standardbibliothek und
  funktioniert auch in Unreal-Python ohne NumPy.
- `import_delivery_props(unreal=None, project_root=None)` importiert die beiden
  verwalteten OBJ-Dateien mit Skalierung 100 als StaticMesh nach
  `/Game/Askir/V80/Props/SM_AskirDeliveryCart` beziehungsweise
  `/Game/Askir/V80/Props/SM_AskirSupplyCrate`.
- `validate_imported_props(unreal, project_root)` prüft vorhandene Pakete ohne
  Neuimport. Beide Engine-Funktionen liefern unter anderem `status`, `assets`,
  `packages` und `source_validation`.

Der Importer setzt ursprüngliche PBR-Materialien explizit aus der MTL auf, da
OBJ-Importer Rauheit und Metall häufig ignorieren. Er erstellt eine einfache
Boxkollision, schreibt OBJ-/MTL-Identität als Asset-Metadaten und prüft tatsächlich
gespeicherte `.uasset`-Dateien, StaticMesh-Typ, LOD0-Sektionen, Materialslots sowie
Z-Achse und Zentimetergrenzen. Der vollständige Quellvalidator bleibt davon
getrennt: sein Erfolg bestätigt keine Engine-Kompilierung und keine Spielbarkeit.
Die Box am Karren ist als solide Interaktions-Requisite gedacht; der offene
Laderaum ist dadurch keine begehbare oder physikalisch beladbare Wagenfläche.
Die Räder sind statische Geometrie, ohne Fahrwerk oder Fahranimation.

Statusnachweise:
`Reports/V80_DELIVERY_PROP_SOURCE_VALIDATION.json`,
`Reports/V80_PROP_SOURCE_PREFLIGHT.json`,
`Reports/V80_PROP_IMPORT.json`.
