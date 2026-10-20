# ASKIR RPG · V82 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Quellprojekt mit Original-Assetquellen. Unreal Engine 5.6 und ein Windows-Buildhost fehlen in dieser Umgebung. Native Kompilierung, Engine-Import und Spielbarkeit sind nicht nachgewiesen. Das Archiv enthält derzeit keine `.umap`, `.uasset` oder Spiel-EXE.**

V82 ergänzt die native Zielerfassung: mittlere Maustaste erfasst einen sichtbaren aktiven Gegner oder löst die Auswahl; `R` wechselt zum nächsten Gegner. Kamera und Figur richten sich zum Ziel, während der Spieler seitlich laufen kann. Eine UMG-Anzeige zeigt Name, Lebenspunkte und Körpermarkierung. Nahkampf und Zauber verwenden bei freier Sicht dieselbe Zielrichtung. Deckung verhindert neue Zielrichtungen sofort; Tod, Menüs, Laden, Reisen und ungültige Ziele lösen die Auswahl.

Normale Angriffe und offensive Magie verwenden gemeinsame Kampfregeln zum Schutz ziviler, inaktiver und toter Figuren. Blocken schützt vor Angriffen aus einem Frontkegel. Native und ältere Guard-Ausdauer werden nicht für denselben Treffer kombiniert. Projektiltreffer berücksichtigen die tatsächliche Flugrichtung; eine Prüfung des Startwegs verhindert das Erscheinen hinter naher Deckung.

V77–V81 bleiben erhalten: Mara-Brunnenmission, Torbens „Die verlorene Lieferung“, Dorf-/Umland-Reise, Händler/Crafting, 24 Zauber, Inventar/Ausrüstung, versionierte Zwei-Bank-Spielstände sowie Hauptmenü und persistente Einstellungen. Originale geriggte Figuren mit Animationen, neun WAV-Dateien sowie Handwagen-/Warenkistenquellen sind enthalten. Die V80-Kartenpfade bleiben bestehen; V82 zählt keine neuen Kartenkopien.

Offline prüfen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v82.py
```

Der Prüfer führt Python-Regressionen, direkt verwendete portable C++-Produktionsregeln mit g++/UBSan sowie binäre Assetprüfungen aus. `Reports/V82_verification.json` enthält die Befehle, tatsächlichen Ergebnisse und geprüften Quelldateihashes. Die ZIP-Erstellung verlangt unveränderte geprüfte Quellen und prüft alle Archiveinträge.

Windows mit UE 5.6, Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV82.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V82"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Die Pipeline sichert vor Änderungen, kompiliert, importiert Assets, erzeugt und validiert beide V80-Karten, führt native Automation aus, erstellt das Win64-Paket und prüft seine Starts. Die Welt-, Titel- und Zielkomponenten-Marker belegen nur den jeweiligen Startzustand. Kamera, Bedienung, Kampf und vollständiges Durchspielen benötigen zusätzliche Engine-Abnahme.

Details: [Änderungen](Docs/V82_CHANGES.md), [Zielerfassung](Docs/V82_TARGETING.md), [Kampfregeln](Docs/V82_COMBAT_POLICY.md), [HUD](Docs/V82_TARGET_HUD.md), [Pipeline](Docs/V82_PIPELINE.md), [Prüfstatus](Docs/V82_VERIFICATION.md). Die vollständige Kampagne, 30 abgenommene Levels und ein getesteter Windows-Build bleiben offen.
