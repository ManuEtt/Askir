# Askir RPG – Rigging und Animationspipeline V4

## 10 entwickelte Schritte
1. Blender-Import der vier vorhandenen Charakter-OBJ-Modelle
2. Gemeinsame humanoide Knochenhierarchie (18 Bones)
3. Zuordnung des Skeletts zu jedem Charaktermodell
4. Automatische, einfache Vertex-Gewichte (Prototyp-Skinning)
5. Idle-Animationsclip
6. Walk-Animationsclip
7. Run-Animationsclip
8. Light-/Heavy-Attack-Clips
9. Block-, HitReact-, Dodge- und Death-Clips
10. FBX-Export pro Charakter und Animationsclip für Unreal Engine 5.6

## So werden echte Rigged-FBX-Dateien erzeugt
Blender 4.x installieren. Im Projektordner ausführen:

`blender -b --python Tools/build_rigged_characters.py`

Das Skript erzeugt nach erfolgreichem Lauf im Verzeichnis
`ArtSource/RiggedCharacters_V4` vier Charakter-FBX und 36 Animations-FBX.

## Unreal Engine 5.6
1. `SK_AskirHero.fbx` als **Skeletal Mesh** importieren, Skeleton neu erstellen.
2. Weitere Charakter-FBX importieren; prüfen, ob dieselbe Skeleton-Hierarchie verwendet wird.
3. Animations-FBX importieren und dem kompatiblen Skeleton zuordnen.
4. Animation Blueprint erstellen, Idle/Walk/Run mit Speed aus `AskirAnimationStateComponent` verbinden.
5. Angriffs-/Block-/Treffer-/Tod-Clips per Montages oder Zustandsmaschine verbinden.
6. Charakter-Blueprints auf die Skeletal Meshes umstellen; Capsule/Collision und Fußhöhe prüfen.

## Grenzen
- Das Blender-Skript ist **erstellt**, aber Blender ist hier nicht installiert. FBX-Dateien wurden **nicht** erzeugt oder in Unreal getestet.
- Die vorhandenen Figuren sind Low-Poly-Blockouts, keine finalen fotorealistischen Modelle.
- Skinning ist eine grobe automatische Zuordnung; Schulter, Ellbogen, Knie, Hände und Füße benötigen manuelle Korrektur.
- Die Bewegungen sind einfache prozedurale Testanimationen, keine Motion-Capture-Qualität.
- Animation Blueprint und Montage-Assets müssen im Unreal Editor eingerichtet werden.
- Für finale Grafik sind hochwertige Meshes, UVs, PBR-Materialien, korrektes Rigging, Retargeting und QA erforderlich.
