# V84 – Originales Heilkräutermodell

Das Kräutervorkommen ist tatsächlich als ursprüngliche 3D-Quelldatei erstellt:
Blender-Projekt, binäres FBX, GLB, trianguliertes OBJ, MTL und eine gerenderte
Blender-Vorschau unter `ArtSource/Environment_V84`. Mesh, Materialien und Vorschau
stehen unter CC0 1.0; Herkunft, sämtliche Dateigrößen und SHA-256-Werte stehen in
`Production/Props/herb_assets_v84.json`. Es wurden keine externen Modelle,
Texturen oder Bilder bezogen.

Die sieben Pflanzen besitzen echte Stängel, geschlossene Blätter und violette
Blüten über einem kleinen Erdklumpen. Das gemeinsame Modell enthält 1.150
Vertices, 1.848 nicht degenerierte Dreiecke und sechs originale PBR-Materialien.
Die Ausdehnung beträgt rund 57,61 × 56,85 × 59,30 cm. Die lokalen Füße liegen
auf Z=0. Blender/OBJ verwenden Meter und Z oben; das GLB verwendet das übliche
glTF-Koordinatensystem. Der Editor importiert das OBJ mit exakt Faktor 100.

Ein gefundener Geometriefehler wurde tatsächlich korrigiert: Die für größere
Wagenbauteile gedachte Kantenabrundung erzeugte an den kleinen Stängeln
zusammenfallende Dreiecke. Die Stängel werden jetzt ohne diese Abrundung gebaut.
Der Generator verweigert bereits vor dem Export degenerierte Dreiecke. Alle
betroffenen Quelldateien wurden neu exportiert und anschließend geprüft.

`Tools/validate_herb_assets_v84.py` liest tatsächliche OBJ-/GLB-Puffer und vergleicht
Vertices, Indices, Dreiecke, Normalen, Materialzuordnung, PBR-Werte, Ursprung und
Maßstab. Hinzu kommen Dateihashes sowie Blender-/FBX-/Vorschauheader.
Der Importvorlauf arbeitet ohne NumPy oder zusätzliche Prozesse im eingebetteten
Editor-Python. Er prüft außerdem OBJ-Indices, Normalen, Dreiecksflächen, sichere
MTL-Verweise und originale Materialwerte vor Editor-Änderungen. 22 neue
Offline-Tests prüfen echte Quellen und gezielt beschädigte Daten sowie
Import-/Reload-Fehlergrenzen.

```bash
python Tools/validate_herb_assets_v84.py
python Tools/import_herb_assets_v84.py --preflight
python -m unittest discover -s Tests -p test_herb_assets_v84.py -q
```

Die Engine-Importfunktion `import_assets(unreal, root)` erzeugt das StaticMesh
`/Game/Askir/V84/Props/SM_AskirHealingHerbs`, setzt die ursprünglichen
Materialkonstanten und eine einfache Box-Kollision, speichert die Pakete und
fordert ein ausdrückliches Paket-Reload an. Danach prüft sie tatsächlichen
Assettyp, Paketdateien, Maßstab, Materialgraphen und Herkunft. Ein leerer
Rückgabewert oder ein fehlgeschlagener Reload zählt nicht als Erfolg.

Diese Unreal-Schritte sind mangels UE 5.6 **nicht ausgeführt**. Die Blender-
Vorschau ist kein Spiel-Screenshot. Die Quest verwendet einen eigenen nativen
Interaktionskörper; die Darstellung und Kollision im Spiel benötigen weiterhin
die dokumentierte Engine-Abnahme.
