# Askir RPG – Charaktermodelle V2

## Ergebnis
Vier **echte statische 3D-Meshes** als GLB und OBJ: Held, Lagerwächter, Händler, Plünderer.
Die Figuren sind maßstäblich etwa 1,9 Meter groß und besitzen unterscheidbare Kleidung und Ausrüstung.
Die neun zuvor erstellten Umgebungsobjekte bleiben enthalten.

## Import
1. Unreal Engine 5.6 Projekt öffnen, Python Editor Script Plugin aktivieren.
2. `Tools/import_realistic_assets.py` über **Execute Python Script** ausführen.
3. `Tools/place_character_previews.py` im gewünschten Testlevel ausführen.
4. Importskalierung prüfen: Modellmaße in Metern, UE nutzt Zentimeter; gegebenenfalls Import Scale 100.
5. Kollision, Materialien, Positionen und Beleuchtung prüfen, Level speichern.

## Wichtige Einschränkungen
Die vier Modelle sind **Blockout-/Low-Poly-Vorschauen**, keine fertigen realistischen Charaktermodelle.
Sie haben **kein Skelett, kein Skinning, keine Animationen, keine PBR-Texturen**.
Sie sind deshalb **nicht als spielbare Skeletal Meshes** an die Charakterklassen angebunden.
Das Platzierungsskript erzeugt nur statische Vorschau-Actors; vorhandene NPC- und Kampfmechanik bleibt unverändert.
Die Modelle können im Editor als visuelle Referenzen verwendet werden.
Für finale Charaktere müssen Topologie, UVs, hochwertige Materialien, Rigging und Animationen neu ausgearbeitet werden.
Ein UE-5.6-Import-/Buildtest wurde in dieser Umgebung nicht durchgeführt.
