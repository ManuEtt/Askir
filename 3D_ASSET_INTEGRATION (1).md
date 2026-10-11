# Askir – Realistic 3D Asset Pack V1

Enthalten: **9 echte, niedrigpolygonale 3D-Meshes** als GLB und OBJ.
Die OBJ-Dateien sind für den Editorimport vorgesehen; GLB-Dateien sind als zusätzliche Austauschformate enthalten.

## Import in Unreal Engine 5.6
1. Das Projekt aus der ZIP entpacken und in Unreal Engine 5.6 öffnen.
2. **Python Editor Script Plugin** aktivieren und Editor neu starten.
3. Im Editor die Python-Datei `Tools/import_realistic_assets.py` ausführen.
4. Die Static Meshes im Content Browser unter `/Game/Askir/Realistic3D_V1` prüfen.
5. In einer separaten Testlevel-Kopie `Tools/place_realistic_assets.py` ausführen.
6. Mesh-Skalierung prüfen: OBJ-Koordinaten sind in **Metern** modelliert; Unreal verwendet **Zentimeter**. Beim Import je nach Interchange-/FBX-Importpfad die Skalierung auf **100** setzen, falls Unreal die Meter nicht automatisch umrechnet.
7. Kollisionsgeometrie (z. B. Auto Convex oder einfache Boxen), Materialien, Beleuchtung und LOD/Nanite nach Bedarf einrichten.

## Enthaltene Assets
- Holzkiste, Fass, Steinmauer, Lagerfeuer, Tanne
- Eisenschwert, Rundschild, Brunnen, Fachwerkhütte

## Entwicklungsstatus und Qualität
- Die Dateien enthalten **echte 3D-Geometrie**, nicht nur Konzeptbilder.
- Es handelt sich um **erste blockout-/low-poly-Modelle mit realistischer Formensprache**, noch **nicht um finale fotorealistische AAA-Assets**.
- Die OBJ-Dateien besitzen keine fertigen PBR-Texturen; GLB enthält einfache Farbwerte.
- Charaktermodelle, Skelette, Skinning und Animationen sind in diesem Paket noch nicht enthalten.
- Unreal Editor und Build konnten hier nicht ausgeführt werden; Import und Platzierung sind vorbereitet, aber **nicht in Unreal getestet**.
- Die Platzierungsroutine erzeugt StaticMeshActors in einem geöffneten Level, verändert aber nicht die C++-Questlogik automatisch.
- Vor Verwendung im finalen Spiel müssen Materialqualität, Collision, UVs, Performance und Asset-Stil geprüft werden.
