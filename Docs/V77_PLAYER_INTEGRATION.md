# V77 – Spieler- und UMG-Integration

Status: Quellcode implementiert; Unreal-Kompilierung und Laufzeitverifikation blockiert.

Der native GameMode erzeugt `AAskirCharacter` und `AAskirPlayerController`, ohne auf den fehlenden Blueprint zu verweisen. Der PlayerController erstellt das native UMG-HUD. Dessen WidgetTree entsteht vor `RebuildWidget`; Anzeigen lesen reale Leben-/Ausdauer-/XP-/Gold-/Missionsdaten. Inventarverwendung, Ausrüstung, Journal, Pause und derselbe Save-Slot wie F5/F9 sind verbunden. Maras Annahme/Abgabe wird erst durch eine Dialogantwort bestätigt.

Enhanced Input erzeugt InputActions und MappingContext zur Laufzeit. Bewegungsrichtung folgt dem Kamerawinkel, Sprint verbraucht Ausdauer. E führt das tatsächliche `IAskirInteractable` auf Loot-/Questactors aus. Die Scherben blockieren den Visibility-Abfragekanal bei erhaltener Pawn-Überlappung. Schwere und leichte Angriffe teilen eine nächste Angriffszeit; Dodge und Block brechen ausstehende Treffer ab. Ausrüstung beeinflusst Waffen- und physischen Rüstungsschutz. Der Cylinder-Fallback ist eine Stellvertretung ohne fertige Animationen.

`RestoreAfterLoad` setzt Angriffe und Eingabestände zurück, aktiviert für lebende Charaktere Gehen und Kollision und stellt für Tote den Todeszustand her. Ein nativer Automation Test wurde ergänzt, aber nicht ausgeführt.

Scherben und Truhen bereiten den Inventarwechsel vor, installieren ohne Delegates Mission, Inventar und persistenten Actorzustand und informieren erst danach Beobachter. Ein Save aus einem Fortschritts-/Inventarcallback erhält damit einen konsistenten Transaktionsstand. Native Audiohooks laden optionale, vom Editor erzeugte V77-Pakete; fehlende Pakete bleiben stumm.

Der Runtime-Bereitschaftsmarker belegt nach einem wirklichen Build lediglich einen initialisierten Spieler. UMG-Bedienung, Navigation, Kampf und Missionsabnahme benötigen weiterhin ihre Engine-/Gameplaytests.
