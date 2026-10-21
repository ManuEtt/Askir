# V83: Gesprächs-Integration und Buildnachweise

V83 verwendet weiterhin die bestehenden Weltpakete `Dornfeld_V80` und `Dornfelder_Umland_V80`. Der neue Editor-Schritt ergänzt Rurik im Dorf; er erzeugt keine zusätzlichen oder umbenannten Levels.

```powershell
.\Scripts\Build-AskirV83.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' -OutputDirectory 'D:\AskirBuilds\V83_01' -CheckOnly
.\Scripts\Build-AskirV83.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' -OutputDirectory 'D:\AskirBuilds\V83_02'
```

Erforderlich sind Windows, Unreal Engine 5.6, Python 3.10 oder neuer und ein neuer Ausgabeordner außerhalb des Projekts. Vor Editor-Änderungen entsteht `Backup/Askir_pre_v83_generation`. `-UseExistingMap` erhält bestehende V80-Weltinhalte. Fehlt Rurik, wird nur er ergänzt; ein vorhandener V83-Rurik wird geprüft. Fremde NPCs, widersprüchliche Identitäten und doppelte Rurik-Instanzen werden nicht ersetzt.

Die geerbte Pipeline kompiliert den Editor und erzeugt oder validiert beide V80-Welten. Danach führt sie `build_conversations_v83.py` in einem zweiten echten Editor-Prozess aus. Dieser setzt den freundlichen nativen `AAskirNPC` mit Label `Dornfeld_Rurik_V83`, stabilem Tag `ASKIR_V83_NPC_RURIK` und `ConversationID=D_DORNFELD_RURIK` an die vorgesehene Position `(500, -1550, 100)`. Die native Guard-Konfiguration verweist auf die vorhandenen V78-Modelle und zwölf Animationen. Torbens Actor und Missionsdaten bleiben erhalten.

Die Position liegt innerhalb der vorhandenen Boden- und Navigationsgrenzen; tatsächliche Zugänglichkeit wird ausschließlich im Editor geprüft. Nach Speichern und erneutem Laden verlangt der Editor-Schritt:

- genau einen korrekt identifizierten Rurik, freundlichen Status und vorhandenen Dialog im tatsächlich kompilierten Katalog;
- gültige Guard-Mesh- und Animationsreferenzen einschließlich nativer Skelett-/Sequenzprüfung;
- echte begehbare Bodenkollision, passende Kapselmaße, freie NPC-Kapsel und freien Interaktionsstandpunkt;
- einen vollständigen Navigationspfad vom vorhandenen Dorfstart zum Interaktionsstandpunkt;
- unveränderte Identität, Klasse und Transformation aller vorher vorhandenen Actors;
- erneute vollständige V80-Validierung beider Welten mit Missions-, Handels-, Navigations- und Übergangsprüfungen.

Das Ergebnis steht in `conversations_editor.json`. Der Runner prüft sowohl den vollständigen V80-Weltbericht als auch die ergänzten V83-Nachweise. Exit-Code 0 oder reine Metadaten genügen nicht. Erst danach folgen alle registrierten nativen `Askir.*`-Tests in beiden Welten, Win64-Paketierung und getrennte Starts von Dorf und Umland.

Jeder paketierte Start verlangt nach der letzten Weltbereitschaft diese drei zusätzlichen Marker, mit zur Welt passender Map und identischem kontrolliertem Spieler:

```text
ASKIR_V80_RUNTIME_READY map=Dornfeld_V80 player=AskirCharacter_0
ASKIR_V81_FRONTEND_READY map=Dornfeld_V80 view=Title
ASKIR_V82_TARGETING_READY map=Dornfeld_V80 player=AskirCharacter_0 component=AskirTargetLockComponent
ASKIR_V83_CONVERSATION_READY map=Dornfeld_V80 player=AskirCharacter_0 component=AskirConversationComponent
```

Die drei zusätzlichen Marker dürfen untereinander in beliebiger Reihenfolge erscheinen. Bereits vorhandene Logs, fremde Spieler, falsche Maps, unvollständige Marker oder fehlende Editor-Nachweise führen zum Abbruch. Der Gesprächsmarker bestätigt nur die konfigurierte native Komponente. Er belegt keinen durchgespielten Dialog.

Der Offline-Vorlauf ist separat ausführbar:

```text
python Tools/build_conversations_v83.py --preflight --report Reports/V83/conversation_source_preflight.json
```

Er prüft die Aktualität des nativen Katalogs, Ruriks Quelldialog und die Boden-/Navigationsgrenzen aus dem vorhandenen Generator. Er erzeugt keine Unreal-Pakete. Fehlen Engine oder Windows, bleibt der Build `Blockiert`. Editor-, Gameplay- und Windows-Nachweise werden dadurch nicht erfüllt. Startup läuft ohne Bild- und Soundausgabe; Dialogbedienung, Speichern/Laden von Gesprächsentscheidungen, Darstellung und vollständiger Spielablauf benötigen eigene Unreal- und manuelle Tests. Alle Gameplay-Abnahmestatus bleiben ausdrücklich `Nicht ausgeführt`.
