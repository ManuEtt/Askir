# V79 · Test- und Abnahmestatus

Die Code-/Assetprüfung wurde am **2026-10-15T18:28:28.172546+00:00** ausgeführt. Der Engine-/Windows-Preflight wurde am 15.10.2026 erneut ausgeführt und blieb blockiert. `Reports/V79_verification.json` enthält Befehle, Ausgaben und SHA256 aller geprüften Quellinputs. Der ZIP-Generator verweigert nachträglich geänderte oder hinzugefügte Quellinputs; der bestätigte Stand wird vollständig mit Dateihashes ausgeliefert.

| Prüfung | Status | Nachweis/Umfang |
|---|---|---|
| Python-Regressionen (einschließlich Quellverträge) | Bestanden | 498 Tests |
| Dornfeld-Produktionsregeln | Bestanden | 73 C++-Assertions, g++/UBSan |
| Geprüfte Gameplay-Arithmetik | Bestanden | 511 C++-Assertions, g++/UBSan |
| Magie-Produktionsregeln | Bestanden | 451 C++-Assertions, g++/UBSan |
| Handel-/Crafting-Produktionsregeln | Bestanden | 146.538 C++-Assertions, g++/UBSan |
| Händlerzustand-Grenzen | Bestanden | 21 C++-Assertions, g++/UBSan |
| Kataloge, Audio und Charakterquellen | Bestanden | 24 Zauber, 5 Handelsitems, 2 Rezepte, 9 WAV, 39 FBX; echte Skin-/Animationsbuffer |
| Unreal-Tests | Blockiert | 34 registriert, 0 ausgeführt |
| Editor/UHT/Projektkompilierung | Blockiert | UE 5.6 fehlt |
| Kartengenerierung, Navigation und Ingame-UI | Blockiert | 0 echte .umap, 0 .uasset |
| Win64-Paket und Start außerhalb des Editors | Blockiert | Engine und Windows-Host fehlen |
| End-to-End-Dorfmission und Kampagne | Nicht ausgeführt | Kein spielbarer Build vorhanden |
| Performance | Blockiert | Keine FPS-/Speicher-/Ladezeitmessung |

Die 147.594 portablen C++-Assertions führen Header aus, die vom Produktionscode verwendet werden. Sie kompilieren **keine** Unreal-Klassen, Blueprints oder UMG-Widgets. Python-Quellverträge prüfen weiterhin nur die erfassten Dateieigenschaften. Grüne Offline-Ergebnisse erfüllen deshalb keine Engine-Spielbarkeitsabnahme.

Neue native Tests decken Händler-Snapshotvalidierung, SaveGame-Serialisierung, echten Handels-/Inventar-Restore, wiederholtes Laden, Transaktionsrückrufe, Ausrüstungsabgleich sowie Interaktionsreichweite, ungültige Konfiguration und die Invalidierung offener Menüs beim Wiederherstellen ab. Ihre Ausführung steht aus.

## Verbindliche Spielabnahme

Keines der vollständigen Spiel-Abnahmekriterien wird in V79 als erfüllt markiert. Insbesondere fehlen nachweislich echte Levels/Unreal-Assets und ein ausführbares Win64-Spiel. Die vorhandene C++-/Python-Implementierung, Kataloge und Originalquellen sind ein Entwicklungsstand.

## Manuelle V79-Abnahme nach erfolgreichem Engine-Build

1. Regulär Dornfeld starten und Mara-Auftrag annehmen, drei unterschiedliche Brunnenscherben sammeln, zwei aktivierte Wächter besiegen, Kellertruhe öffnen und bei Mara genau 90 Gold und 180 XP abholen.
2. Drei neue Ressourcen-Pickups aufnehmen. Sie dürfen Maras Scherbenzähler nicht verändern.
3. An der Werkbank einen Heiltrank und ein Jagdschwert herstellen; Zutaten müssen einmal verbraucht und Ergebnisse einmal hinzugefügt werden. Fehlende Zutaten, volle Slots, gesperrte Rezepte und ungültige Mengen müssen ohne Teiländerungen scheitern.
4. Heiltrank verwenden und Schwert ausrüsten. Bei Edda kaufen und verkaufen; beide Geldkonten, Inventar und Bestand müssen gemeinsam reagieren. Nach Verkauf des letzten ausgerüsteten Schwerts darf sein Bonus nicht bestehen bleiben.
5. Wiederholt speichern, weiterhandeln und denselben Save laden. Bestände und Konten müssen exakt zurückkehren. V1–V4-Migration, fehlende V5-Belege und beschädigte Daten separat prüfen.
6. Während geöffneter Menüs sterben, Händler entfernen oder Kontext wechseln. Veraltete Buttons dürfen keine weiteren Transaktionen, Gegenstandsverwendungen oder Zauberwechsel auslösen.
7. Tatsächlichen Win64-Build außerhalb des Editors starten, Mission komplett spielen und FPS, RAM sowie Ladezeiten auf der verfügbaren Hardware messen.

Alle sieben manuellen Punkte stehen auf **Nicht ausgeführt**.
