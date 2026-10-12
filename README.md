# ASKIR RPG · V77 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Gesicherter Entwicklungsstand. Unreal-Kompilierung, Spielbarkeit und Windows-Build sind noch nicht nachgewiesen: Hier fehlen Unreal Engine 5.6 und ein Windows-Host. Das Archiv enthält weiterhin keine `.umap` oder `.uasset`.**

V77 repariert „Das Flüstern im Brunnen“, persistente Scherben-/Truhenbelege, Spieler-Killzuordnung, gestufte Wächter, einmalige 90-Gold-/180-XP-Abgabe, Questjournal, Save-Format 3 mit zwei Sicherungsbänken, Levelwechsel und Inventar-/Handels-/Craftingtransaktionen. Enhanced Input und native UMG-Anzeigen für Status, Interaktionshinweise, Maras bestätigbaren Dialog, Inventar, Journal und Pause sind im Quellcode integriert. Diese Unreal-Integrationen sind noch unkompiliert.

Sieben tatsächlich erzeugte WAV-Dateien, ein Unreal-Python-Dorfkartengenerator und eine Windows-Build-/Testpipeline sind enthalten. Der Generator speichert und prüft echte Editor-Dateien, bevor er den Startlevel umstellt. Er wurde hier mangels Engine nicht ausgeführt.

Verfügbare Prüfungen:

```bash
python Tools/verify_v77.py
python Tools/generate_audio_v77.py --validate-only
```

Die C++-Regelprüfung benötigt g++ mit C++17 und UBSan. `Reports/V77_verification.json` trennt Offline-Tests von blockierten Unreal-/Windows-Tests.

Windows mit UE 5.6, den erforderlichen Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV77.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V77"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Ablauf: Sicherung → Editor-Kompilierung → Dorfkarte/Assets → Save/Reload-Prüfung → native Automation → Windows-Paket → Startprüfung. Ein Starttest ersetzt kein manuelles Durchspielen. Weitere Hinweise: [Start und Grenzen](Docs/V77_START_AND_LIMITATIONS.md), [Pipeline](Docs/V77_UNREAL_PIPELINE.md).

Der [V76-Audit](Docs/V77_TECHNICAL_AUDIT.md) erfasst die unveränderte Ausgangsbasis. Änderungen und Grenzen stehen in `Docs/V77_*`; aktuelle Prüfergebnisse in `Reports/V77_verification.json`. Die alte README liegt unverändert in `Docs/README_V76_ARCHIVE.md`.

Die vollständige Kampagne, 30 spielbare Levels, produktionsreife Animationen und der Windows-Build bleiben offen. GitHub verweigerte den Branch-Upload der verbundenen Integration mit HTTP 403; der lokale Commit und das Projekt-ZIP sichern den Stand.
