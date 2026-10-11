# Askir V60 — GitHub-Abgleich und Reparaturen

## Vergleich
Repository: `ManuEtt/Askir`, Branch `main`, letzter bei Prüfung sichtbarer Commit `63a9b9ad3a853c6c85411bca095c59abad8f5ae1` (11.10.2026, 21:33).
Folgende fünf Dateien waren **byteidentisch** (Git-Blob-SHA-1) zwischen GitHub und dem V60-ZIP:
- `README_V60.md`
- `Source/Askir/AI/AskirBossAIController.h`
- `Source/Askir/Characters/AskirCharacter.cpp`
- `Production/Bosses/boss_animation_profiles_v60.json`
- `Tools/configure_combat_profiles_v55.py`

Ein vollständiger Vergleich *aller* GitHub-Dateien war über den vorhandenen Connector nicht möglich; deshalb keine Behauptung einer vollständigen Synchronität.

## Gefundene und korrigierte Fehler
1. **V54-Validator** verwendete falsches Generated-Class-Format (`.../BP_X_C` statt `.../BP_X.BP_X_C`). Das erzeugte eine reproduzierbare Fehlermeldung in der bestehenden Testsuite. Jetzt korrigiert.
2. **V55-Kampfprofil-Editor-Skript** setzte Controller-Eigenschaften am Pawn-CDO (`AAskirCharacter`), der diese Felder nicht besitzt. Jetzt werden die sechs Controller-BP-CDOs konfiguriert; korrekte UE-Property-Namen und Asset-Pfade werden verwendet.
3. **V56-Boss-Blueprint-Konflikt**: V53 erzeugte `BP_AI_AskirBoss` als Kind des allgemeinen KI-Controllers. V56 übernahm die bereits existierende Klasse ohne Reparenting, wodurch die Bossphasen ausfielen. Jetzt wird separat `BP_AI_AskirBossPhases` mit der richtigen nativen Elternklasse erstellt, dessen Parent geprüft und der Boss-Pawn darauf umgestellt. Vorhandene Assets bleiben unverändert.
4. Boss-Kampfprofilreferenzen auf den neuen Controllerpfad angepasst. Vor Anwendung zuerst `setup_boss_ai_v56.py`, dann `configure_combat_profiles_v55.py` ausführen.
5. Fünf Regressionstests ergänzt.

## Ausführung in Unreal
1. Sicherheitskopie des Projekts anlegen, C++ in Unreal Engine 5.6 kompilieren.
2. V51/V53 Blueprint-Generierung sicherstellen.
3. `Tools/setup_boss_ai_v56.py` im Unreal-Python-Editor zunächst mit `DRY_RUN=True` ausführen; dann bewusst `False` setzen.
4. `Tools/configure_combat_profiles_v55.py` ebenfalls erst im Preview, dann bewusst anwenden.
5. AIControllerClass auf `BP_AskirBoss` und Controller-Properties im Editor überprüfen.
6. Gameplay-Test: Boss-Phasen, Angriffsabstände, Angriffstimer und Stagger.

## Grenzen
Offline-Unittests sind kein UE-Compile und kein Laufzeittest. Die ZIP ist eine **korrigierte lokale Fassung**, keine Änderung am GitHub-Branch. Assets im Editor werden nicht automatisch erzeugt, solange `DRY_RUN=True` ist.
