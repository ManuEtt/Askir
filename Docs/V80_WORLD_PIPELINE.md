# V80: Dornfeld und Dornfelder Umland

Die V80-Pipeline ergänzt den vorhandenen Dorf-Blockout um die angebundene Bauernstraße und die kanonische Nebenquest `SQ_001` von Torben. Die Geschichte und Belohnung stammen aus `Production/Quests/sidequests.json`: zwei Banditen besiegen, Fracht sichern, zu Torben zurückkehren; 100 XP, 16 Gold und einmal `item_002`.

**Aktueller Nachweis:** Quellenprüfung möglich; Editor-Generierung, Unreal-Kompilierung, Navigation in Unreal, Reise im laufenden Spiel und Windows-Paketierung sind in dieser Linux-Umgebung ohne UE 5.6 blockiert. Es wurden keine `.umap`- oder `.uasset`-Dateien erzeugt. Die beiden Karten zählen daher noch nicht als abgenommene spielbare Levels.

## Ausführen

Die unabhängige Quellenprüfung erzeugt keine Unreal-Assets:

```bash
python Tools/build_dornfeld_v80.py --preflight --report Reports/V80/world_source.json
python -m unittest Tests.test_world_pipeline_v80
```

Auf einem Windows-Rechner mit Unreal Engine 5.6, C++-Buildwerkzeugen und Python 3.10+:

```powershell
python -m pip install -r requirements-dev.txt
./Scripts/Build-AskirV80.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' -OutputDirectory 'D:\AskirBuilds\V80_001'
```

`OutputDirectory` muss neu sein und außerhalb des Projektordners liegen. Der Runner sichert die Quellen vor Editor-Änderungen, kompiliert `AskirEditor`, startet die echte Editor-Python-Generierung, führt registrierte native Automationstests in beiden Karten aus, paketiert beide Karten und kontrolliert den Start der EXE mit und ohne Kartenparameter. `-CheckOnly` prüft lediglich die benötigte Host-/Engine-Umgebung. `-UseExistingMap` erhält beide vorhandenen V80-Karten und prüft ihre gespeicherten Inhalte; vorhandene V80-Karten werden sonst ausdrücklich vor Überschreiben geschützt.

## Gespeicherte Karten und Prüfgrenzen

| Paket | Inhalt | Verbindung |
|---|---|---|
| `/Game/Askir/Maps/Dornfeld_V80` | Erhaltene Brunnenmission, Händler/Werkbank und Torben | `DORNFELD_TO_UMLAND_V80` |
| `/Game/Askir/Maps/Dornfelder_Umland_V80` | Bauernstraße, Banditenlager und interaktive Lieferung | `UMLAND_TO_DORNFELD_V80` |

Beide Tore reisen über die Interaktionstaste E. Das Ziel benennt das jeweils andere Kartenpaket und dessen stabile Portal-ID. Der Eintrittspunkt liegt 420 cm auf der sicheren Seite des Tores, außerhalb des Triggerbereichs einschließlich der nativen Spielerkapsel. Die Portale selbst sind nicht blockierend. Die Bauernhäuser sind solide Blockout-Geometrie mit sichtbaren Fassaden; sie besitzen keine begehbaren Innenräume.

Die Originalmodelle `SM_AskirDeliveryCart` und `SM_AskirSupplyCrate` werden aus den OBJ-Quellen unter `ArtSource/Delivery_V80` in Zentimeter importiert. Der Fracht-Aktor behält die logische Interaktionsposition; sein untergeordnetes `Visual` wird auf den Boden versetzt. Der separate Wagen ist ein sichtbares Objekt mit Kollision. Bei einer fehlgeschlagenen Modellprüfung gibt es keinen stillen Rückfall auf einen Würfel.

Die Startkarte wird erst nach allen folgenden Editor-Prüfungen auf V80 umgestellt:

- Beide real gespeicherten Dateien existieren, laden als `UWorld` und können erneut im Editor geöffnet werden.
- Native GameMode-Auswahl, reziproke Portal-IDs, Ziele, Ankunftspositionen, Gegner-/Relaisreferenzen, Cargo-Rolle und Reward-Paket stimmen nach dem Laden.
- Boden-Traces und die tatsächlichen Kapselmaße des nativen Spielers bestätigen freie Start-/Ankunftspositionen.
- Vollständige Navigationspfade führen vom jeweiligen Start zu Portal und Questobjekten sowie vom Ankunftspunkt zum Start. Die erhaltene Brunnenmission und Ökonomie werden zusätzlich geprüft.
- Materialien, Gegenstände, Audio, drei Skelettmodelle mit jeweils zwölf kompatiblen Animationen sowie die originalen Lieferungsmodelle besitzen gespeicherte Assetpakete.

`UAskirEditorLevelTools` kapselt dafür native WorldSettings-, Brush- und Kollisionsfunktionen, die in Python nicht zuverlässig als Editor-API verfügbar sind. Die Pipeline verwendet `EditorLevelLibrary.new_level/load_level/save_current_level`, `EditorActorSubsystem`, `UnrealEditorSubsystem.get_editor_world` und `NavigationSystemV1.find_path_to_location_synchronously`. Diese Aufrufe sind hier statisch geprüft; ihre erfolgreiche Ausführung in UE 5.6 ist noch offen. Der native Navigationsaufbau allein zählt nicht als Erfolg: fehlende oder partielle Pfade brechen die Generierung ab.

`world_editor.json` trennt Editor-Prüfung von Laufzeitprüfung; `gameplay_status` bleibt `Nicht ausgeführt`. Die Python-Tests verwenden temporäre Byte-Dateien ausschließlich, um unvollständige/falsche Nachweise zurückzuweisen. Sie laden keine Unreal-Karte und weisen keine Spielbarkeit nach. Auch ein erfolgreicher späterer EXE-Start ist nur ein Starttest; das vollständige Durchspielen beider Missionen, Speichern/Laden während Reisen, Sichtprüfung, Animationen und Performance erfordern gesonderte Laufzeit-QA.
