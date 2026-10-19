# V80 · Tatsächliche Prüfungen und Abnahme

Offline-Ausführung: **2026-10-18T17:51:21.409953+00:00**. Vollständige Befehle, Rückgabecodes, Ausgaben und SHA256 der 513 geprüften Quellen liegen in `Reports/V80_verification.json`. Der zusätzliche Win64-Preflight vom 18.10.2026 liegt in `Reports/V80/pipeline_environment.json`.

| Prüfung | Status | Umfang |
|---|---|---|
| Python-Regressionen, Quellverträge und Artefaktgrenzen | Bestanden | 555 Tests |
| Dornfeld-Produktionsregeln | Bestanden | 73 portable C++-Assertions |
| Gameplay-Arithmetik | Bestanden | 511 portable C++-Assertions |
| Magie-Produktionsregeln | Bestanden | 451 portable C++-Assertions |
| Handel und Crafting | Bestanden | 146.538 portable C++-Assertions |
| Händlerzustand | Bestanden | 21 portable C++-Assertions |
| Lieferungszustände und Belege | Bestanden | 1.660 portable C++-Assertions |
| Reise- und Ankunftsregeln | Bestanden | 2.471 portable C++-Assertions |
| Questbelohnung und XP | Bestanden | 3.521 portable C++-Assertions |
| WAV, Rigs und Animationen | Bestanden | 9 WAV, 39 FBX, echte Skin-/Animationsbuffer |
| Originale Lieferungsmodelle | Bestanden | 2 GLB/OBJ/BLEND-Modelle, 7.764 Dreiecke, Materialien und Provenienz |
| Editor-Quellen und Berichtsgrenzen | Bestanden | Beide Kartenverträge, Gegenrouten, Paket-/Nav-/Kollisionsnachweise werden geprüft |
| UHT/Unreal-C++-Kompilierung | Blockiert | Unreal Engine 5.6 fehlt |
| Native Unreal-Automation | Blockiert | 46 registriert, 0 ausgeführt |
| Kartengenerierung, Assetimport und echte Navigation | Blockiert | 0 .umap und 0 .uasset erzeugt |
| End-to-End-Dorfmission und Lieferungsquest | Nicht ausgeführt | Kein ausführbarer Spiel-Build vorhanden |
| Windows-Paket und EXE-Start | Blockiert | Linux-Host, keine UE-/Windows-Buildumgebung |
| Performance | Blockiert | Keine FPS-, Spielspeicher- oder Ladezeitmessung |

Die **155.246 portablen C++-Assertions** wurden mit C++17, `-Wall -Wextra -Werror -pedantic` und UndefinedBehaviorSanitizer ausgeführt. Sie verwenden unmittelbar die Header der Produktionsimplementierung. Sie kompilieren keine Unreal-Klasse und führen keinen Editor aus. Python-Tests für Editorberichte verwenden bewusst künstliche temporäre Dateien, um falsche oder unvollständige Nachweise abzulehnen; sie bestätigen keine echte Karte.

Neue native Tests prüfen Lieferungsannahme, einmaligen Fraktionsruf, reale Spieler-Killzuordnung, verspätete Relay-Bindung, Frachtsicherung, SaveGame-Serialisierung und Wiederherstellung, mehrfachen Abschluss, volle Taschen, XP-Überlauf, widersprüchliche Belege, Snapshot-GC, abgewiesene Reiseanfragen, eindeutige Komponenten und Menü-Eingabesperren. Diese Tests sind registriert, **nicht in Unreal ausgeführt**.

Die Modellquellen wurden zusätzlich mit Blender 4.3.2 erzeugt und ihre tatsächlichen Vorschauen visuell geprüft. Die Objekt-/GLB-Prüfung liest die binären Geometrie- und Materialdaten und vergleicht sämtliche Dreiecke. Der Importer bleibt Engine-blockiert.

## Verbindliche Abnahmekriterien

Keines der vollständigen Spiel-Abnahmekriterien wird als erfüllt markiert. Insbesondere existieren weiterhin keine echten Levels, Unreal-Assets oder Windows-Spiel-EXE. Die beiden geplanten V80-Karten zählen nicht als fertige spielbare Levels. Die gesamte Kampagne, drei Enden und mindestens 20 vollständig spielbare Nebenquests bleiben offen.

## Ausstehende Engine- und Paketprüfung

1. V80-Buildpipeline auf einem tatsächlichen Windows-/UE-5.6-Rechner ausführen; die gespeicherten Karten und alle importierten Assets laden.
2. „Das Flüstern im Brunnen“ komplett spielen und genau einmal 90 Gold/180 XP erhalten.
3. Torben zunächst ablehnen, dann eine Entscheidung bestätigen. Ruf darf bei erneuter Interaktion oder Laden nicht zusätzlich steigen.
4. Ins Umland reisen, je einen Banditen mit Spielerangriff besiegen und zwischen den Kills speichern/laden. Andere Killer und doppelte Todesrückrufe dürfen keinen falschen Fortschritt erzeugen.
5. Die Fracht nach beiden Kills sichern; Rückreise muss Dorfquest, Warenbestände, Inventar und Ruf erhalten.
6. Torben den Auftrag bestätigen. 100 XP, 16 Gold und einmal `item_002` müssen gemeinsam eintreffen. Volle Taschen und unzulässige XP müssen ohne Teiländerungen scheitern.
7. Vor/nach jedem Ziel sowie in beiden Regionen wiederholt speichern/laden. Zielbelege, Gegner, Kiste und Belohnungen müssen der geladenen Zeitleiste entsprechen.
8. Fehlenden Ziellevel, fehlende/mehrdeutige Ankunft, blockierte Ankunft, gescheiterten Reisebeginn und Wiederholung/Rückkehr testen. Bis zur Wiederherstellung dürfen keine konkurrierenden Transaktionen oder Schäden eintreten.
9. Das fertige Win64-Paket außerhalb des Editors starten, beide Missionen durchspielen und Performance auf der verfügbaren Hardware messen.

Alle neun manuellen Punkte: **Nicht ausgeführt**. Engineabhängige Schritte sind durch die dokumentierte Umgebung blockiert.

## Versionssicherung

V80 wird in der lokalen Git-Historie und separaten Projekt-ZIPs gesichert. Ein Upload dieser Version zu GitHub wurde nicht ausgeführt; der frühere Repository-Review dokumentiert HTTP 403 beim Branch-Upload. Es wird kein neuer Remote-Commit behauptet.
