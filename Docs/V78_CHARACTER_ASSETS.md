# V78 – Originale Charakter- und Animationsquellen

Drei tatsächlich erzeugte, stilisierte Low-Poly-Figuren liegen unter
`ArtSource/Animated_V78`: Held, Wache und Räuber. Sie unterscheiden sich durch
Kleidung, Materialien und gebundene Ausrüstung. Der Held trägt einen kurzen
Mantel und Lederteile, die Wache Metallrüstung und Schild, der Räuber eine
dunkle Gesichtsdeckung und Brustplatte. Es handelt sich um einfache
prozedurale Kunstquellen, nicht um fertige Produktionscharaktere.

| Figur | Quellvertices | Quellpolygone | Bones | Animationsclips |
|---|---:|---:|---:|---:|
| Hero | 1.156 | 964 | 18 | 12 |
| Guard | 1.212 | 1.016 | 18 | 12 |
| Raider | 1.128 | 944 | 18 | 12 |

Jede Figur enthält eine editierbare `.blend`, eine echte skinned `.glb`,
eine Mesh-/Skeleton-`.fbx` und zwölf einzelne Animations-`.fbx`-Dateien.
Insgesamt sind 39 FBX-Dateien vorhanden. Die Animationen heißen Idle, Walk,
Run, Sprint, Jump, Dodge, LightAttack, HeavyAttack, Hit, Death, Block und Cast.
Jede besitzt Schlüsselkurven, deformierende Bones und tatsächliche Vertexgewichte.
Ober- und Unterarme sowie Beine verwenden gemeinsame Gelenkringe mit gemischten
Gewichten; Gürtel, Mantel und Ausrüstung folgen demselben Armature-Modifier.

`Tools/build_animated_characters_v78.py` wurde mit Blender 4.3.2 ausgeführt.
Die Quelldaten verwenden Meter, Körperhöhe etwa 1,80 m, Füße bei Z=0,
Blender-Z als Hochachse und Blickrichtung +X. FBX schreibt die Einheitentransformation
für den Unreal-Import, ohne Leaf Bones oder mehrere Actions pro Datei.
Der Unreal-Importer muss die tatsächliche Zentimeterskalierung und Ausrichtung
abschließend prüfen; `MeshRotation` in der Playback-Komponente erlaubt eine Korrektur.

Alle 36 Clips wurden in Blender an mehreren Zeitpunkten auf dem ausgewerteten
Skin getestet. Jeder veränderte tatsächliche Vertexpositionen. Danach wurden
alle drei GLBs erneut in Blender eingelesen: jeweils 18 Bones, zwölf Actions
und ein echtes Mesh mit Armature-Modifier. Der Bericht steht in
`Reports/V78_CHARACTER_SOURCE_BLENDER.json`.

Eine zweite Prüfung in `Tools/validate_animated_sources_v78.py` liest ohne
Blender und Unreal die GLB-Binäraccessors. Sie prüft alle Joint-Indizes,
normalisierte Skin-Gewichte, 18 nicht singuläre Inverse-Bind-Matrizen,
Animationszeiten und die zwölf Clipnamen. Aus den tatsächlichen GLB-Kanälen
berechnet sie Gelenkmatrizen und gewichtete Vertexpositionen. Auch diese
unabhängige CPU-Skin-Prüfung bestand für alle 36 Clips. Die gemessene größte
Bewegung je Clip reicht von ungefähr 1,9 cm bei Idle bis 2,37 m beim Fallen.
Ergebnisse stehen in `Reports/V78_CHARACTER_SOURCE_BINARY.json`.
Für diese unabhängige Prüfung ist NumPy erforderlich.

Die 39 FBX-Quelldateien bestanden außerdem die Header-, Größen- und
Prüfsummenprüfung des Importers (`Reports/V78_ANIMATION_SOURCE_PREFLIGHT.json`).
Diese Prüfung bescheinigt keine erfolgreiche Unreal-Einfuhr. Der tatsächliche
Unreal-Import, native Kompilierung, Darstellung im Spiel und Cook sind
**nicht ausgeführt**, weil UE 5.6 in dieser Umgebung fehlt. Die Quellen ersetzen
noch keine `.uasset`-Pakete im ausgelieferten Projekt.

Geometrie, Hautgewichte, Farben und Bewegungsdaten sind eigene prozedurale
Erzeugnisse dieser Entwicklung. Externe Modelle, Texturen und Motion-Capture-Daten
wurden nicht verwendet. Der Asset-Ordner enthält eine CC0-1.0-Widmung und die
Herkunftsübersicht; die Lizenzen von Blender und Unreal bleiben eigenständig.
Die Vorschau-PNGs werden in Blender gerendert und sind keine Unreal-Spielbilder.

Es fehlen Gesichtsausdrücke, Fingerbones, hochwertige Texturen, Fuß-IK,
Animationsblends, physikalische Kleidung und auswechselbare Waffensockets.
Beim einfachen Skin können Gelenke stark verformt werden. Die Bewegungen müssen
in Unreal visuell geprüft und für ein fertiges Spiel weiter ausgearbeitet werden.
