# V78 – Skeletale Laufzeit-Animationen

`UAskirAnimationPlaybackComponent` verbindet die ursprünglichen V78-Charakterquellen
mit `ACharacter::GetMesh()`. Die Komponente ersetzt die vorherige reine Zustandsanzeige
durch native Single-Node-Wiedergabe tatsächlicher `UAnimSequence`-Assets. Ein Animation
Blueprint ist für diese Pipeline nicht erforderlich. Angelegte Soft-Asset-Pfade sind
keine bereits importierten Unreal-Assets.

## Quell- und Importvertrag

Die Figuren `Hero`, `Guard` und `Raider` haben jeweils eine Mesh-FBX-Datei
`ArtSource/Animated_V78/Askir_<Rolle>.fbx` und zwölf separate Animationsdateien
`Askir_<Rolle>_<Clip>.fbx`. Die Clips heißen `Idle`, `Walk`, `Run`, `Sprint`,
`Jump`, `Dodge`, `LightAttack`, `HeavyAttack`, `Hit`, `Death`, `Block` und `Cast`.
Separate FBX-Dateien verhindern mehrdeutige Namen und die versehentliche Einfuhr
mehrerer Blender-Actions in einen einzigen Unreal-Clip.

`Tools/import_animated_assets_v78.py` muss im kompilierten Unreal-5.6-Editor laufen.
Er legt echte Pakete unter `/Game/Askir/CharactersV78/<Rolle>/` an:
`SK_Askir<Rolle>`, das tatsächliche Skeleton und `A_<Clip>`. Jeder Clip wird explizit
auf dem Skeleton seines Meshes importiert. Vor Importbeginn werden alle 39 benötigten
FBX-Quelldateien geprüft. Der Importer überschreibt vorhandene Pakete nicht, prüft
gespeicherte `.uasset`-Dateien und lehnt fremde Skeletons sowie leere oder nicht endliche
Animationslängen ab. Der Importbericht beschreibt Asset-Prüfungen; er bescheinigt
keine erfolgreiche Spielanimation.

Außerhalb von Unreal kann `python Tools/import_animated_assets_v78.py --preflight
--report Reports/V78_ANIMATION_SOURCE_PREFLIGHT.json` ausschließlich Quellheader,
Dateigrößen und Prüfsummen prüfen. Ohne Unreal ist die tatsächliche Asset-Generierung
als `Blockiert` ausgewiesen.

## Darstellung und Ablauf

Die Standardfigur ist `Hero`. `ConfigureCharacterVisual("Guard")` beziehungsweise
`ConfigureCharacterVisual("Raider")` kann vor `TryInstallVisual()` einen anderen Satz
wählen. Alle zwölf Clips und das Mesh müssen vorhanden sein und dasselbe Skeleton
besitzen, bevor die Komponente irgendeine Darstellung verändert. Fehlende Pakete
werden ohne Load-Versuch übersprungen. Designerzugewiesene andere Skeletal Meshes
bleiben erhalten, sofern `bReplaceExistingMesh` nicht aktiviert wurde.

Das importierte Mesh benötigt Zentimeter, Füße bei Z=0 und Blickrichtung +X.
Die Komponente verschiebt es um die lokale Kapsel-Halbhöhe nach unten; Rotation
und Skalierung sind konfigurierbar. Kollision verbleibt auf der Character-Kapsel.
Nach erfolgreicher Installation kann der Besitzer seinen statischen Ersatzkörper
ausblenden. Dynamische Pfade müssen im Cook enthalten sein; dafür ist
`/Game/Askir/CharactersV78` als Always-Cook-Verzeichnis vorgesehen.

Tod hat Vorrang vor allen Aktionen. Angriff, Ausweichen, Trefferreaktion und Zaubern
haben Vorrang vor Bewegung. Blocken läuft bis zum Loslassen. Idle, Walk, Run und
Sprint loopen; wiederholte gleichartige Aktionen starten über `ActionSequence` neu.
Die Laufzeit einer vorübergehenden Aktion folgt dem importierten Clip. Tod und
anhaltende Sprungzustände halten ausdrücklich das letzte Bild. `RestorePlaybackAfterLoad()`
wird nach dem Laden des Animationszustands aufgerufen; ein gespeicherter toter Charakter
zeigt sofort das Tod-Endbild. Nach Wiederbelebung wählt der zurückgesetzte
Animationszustand wieder die passende Bewegung.
Der bestehende Timer für Nahkampftreffer bleibt unabhängig von AnimNotifies.

## Prüfung und verbleibender Status

Offline-Tests prüfen insbesondere falsche Skeleton-Zuordnung, leere Clipdauer,
unvollständige Quellen, unsichere Paketpfade und fehlende tatsächliche Asset-Dateien.
Native Automation-Tests prüfen Aktionsprioritäten, Bewegungszustände und den Erhalt
vorhandener Darstellung bei fehlenden importierten Assets. Diese nativen Tests
benötigen UE 5.6 und sind ohne Engine **nicht ausgeführt**.

Diese Pipeline enthält keine Übergangsblends, Root-Motion-Auswertung, Fuß-IK,
Retargeting oder Ausrüstungs-Socket-Animation. Skinning, Ausrichtung, reale
Animationswiedergabe, sichtbare Qualität und Cook müssen mit UE 5.6 geprüft werden,
bevor sie als funktionierend oder spielbar gelten.
