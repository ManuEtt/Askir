# ASKIR RPG · V78 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Gesicherter Entwicklungsstand. Unreal-Kompilierung, Spielbarkeit und Windows-Build sind noch nicht nachgewiesen: Hier fehlen Unreal Engine 5.6 und ein Windows-Host. Das Archiv enthält weiterhin keine `.umap` oder `.uasset`.**

V78 ergänzt die Laufzeitlogik für die 24 kanonischen Zauber, Mana/Schild und Abklingzeiten, Save-Format 4 mit Migration älterer Spielstände, F/Mausrad/K-Steuerung und eine Magieauswahl im UMG. Neue originäre Blender-Charakterquellen enthalten echte Rigs und zwölf Animationen pro Figur; ein Unreal-FBX-Importer und native Playback-Komponente sind enthalten. Unreal-Import und Wiedergabe sind noch nicht ausgeführt.

Der erhaltene V77-Stand repariert „Das Flüstern im Brunnen“, persistente Scherben-/Truhenbelege, Spieler-Killzuordnung, gestufte Wächter, einmalige 90-Gold-/180-XP-Abgabe, Questjournal, Save-Format 3 mit zwei Sicherungsbänken, Levelwechsel und Inventar-/Handels-/Craftingtransaktionen. Enhanced Input und native UMG-Anzeigen für Status, Interaktionshinweise, Maras bestätigbaren Dialog, Inventar, Journal und Pause sind im Quellcode integriert. Diese Unreal-Integrationen sind noch unkompiliert.

Neun tatsächlich erzeugte WAV-Dateien, ein Unreal-Python-Dorfkartengenerator und eine V78-Windows-Build-/Testpipeline mit Characterimport sind enthalten. Der Generator speichert und prüft echte Editor-Dateien, bevor er den Startlevel umstellt. Er wurde hier mangels Engine nicht ausgeführt.

Verfügbare Prüfungen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v78.py
python Tools/generate_audio_v77.py --validate-only
```

Die C++-Regelprüfung benötigt g++ mit C++17 und UBSan. `Reports/V78_verification.json` trennt Offline-Tests von blockierten Unreal-/Windows-Tests.

Windows mit UE 5.6, den erforderlichen Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV78.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V78"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Ablauf: Sicherung → Editor-Kompilierung → Dorfkarte/Assets → Save/Reload-Prüfung → native Automation → Windows-Paket → Startprüfung. Ein Starttest ersetzt kein manuelles Durchspielen. Weitere Hinweise: [Start und Grenzen](Docs/V77_START_AND_LIMITATIONS.md), [Pipeline](Docs/V78_CHANGES.md).

Der [V76-Audit](Docs/V77_TECHNICAL_AUDIT.md) erfasst die unveränderte Ausgangsbasis. Änderungen und Grenzen stehen in `Docs/V77_*`; aktuelle Prüfergebnisse in `Reports/V78_verification.json`. Die alte README liegt unverändert in `Docs/README_V76_ARCHIVE.md`.

Die vollständige Kampagne, 30 spielbare Levels, produktionsreife Animationen und der Windows-Build bleiben offen. GitHub verweigerte den Branch-Upload der verbundenen Integration mit HTTP 403; der lokale Commit und das Projekt-ZIP sichern den Stand.
