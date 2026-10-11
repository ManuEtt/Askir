# Askir RPG V19 – 3D-Modelle und Unreal-Integration

## Tatsächlich erstellt
- 13 verbesserte GLB-Modelle unter `ArtSource/Enhanced3D_V19/`, zusätzlich OBJ-Dateien.
- Hütte: Holzbalken und Fenster; Fass: Dauben und Eisenreifen; Kiste: Verstrebungen; Mauer: Fugen; Brunnen: Winde und Seil; Baum: Äste; Lagerfeuer: Holz und Steine; Waffen und Charaktere: zusätzliche Details.
- Reproduzierbarer Modellgenerator: `Tools/build_detailed_models_v19.py`.
- Blender-FBX-Export: `Tools/export_enhanced_fbx_blender.py`.
- Unreal-FBX-Import: `Tools/import_enhanced_fbx_unreal.py`.
- Unreal-Testdorf-Platzierung: `Tools/place_enhanced_village_unreal.py`.

## Integration unter Windows
1. ZIP entpacken, Blender 4.x installieren.
2. Im Projektordner `blender -b --python Tools/export_enhanced_fbx_blender.py` ausführen.
3. Unreal Engine 5.6 starten, Python Editor Script Plugin aktivieren.
4. Die Datei `Tools/import_enhanced_fbx_unreal.py` im Unreal Python-Editor ausführen (als Datei, damit `__file__` korrekt gesetzt ist).
5. Testlevel öffnen, `Tools/place_enhanced_village_unreal.py` ausführen, Level speichern.
6. Skalierung, Materialien, Kollisionen und Beleuchtung kontrollieren. Materialien sind einfache Vertexfarben; FBX kann diese je nach Import-Einstellungen anders behandeln.

## Wichtige Grenzen
- Die Modelle sind weiterhin stilisierte Low-Poly-Prototypen, keine fertigen fotorealistischen Produktionsassets mit PBR-Texturen.
- Die vier Charaktere sind **statische Vorschau-Geometrien**. Zusatzdetails sind noch nicht geriggt oder gewichtet; V4-Rigging bleibt eine ungetestete separate Pipeline.
- Blender und Unreal sind in der Ausführungsumgebung nicht installiert. FBX-Export, Unreal-Import, Kompilierung und PIE wurden **nicht** getestet.
- Unreal `.uasset` und `.umap` Dateien können hier nicht erzeugt werden. Die Editor-Skripte übernehmen Import und Platzierung erst beim lokalen Ausführen.
