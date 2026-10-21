# V83 – Prüfstatus und offene Abnahme

Maßgeblich ist `Reports/V83_verification.json` mit tatsächlichen Befehlen,
Ergebnissen und unveränderten Quelldateihashes. Der erneute Engine-Vorlauf unter
`Reports/V83/pipeline_environment.json` meldet fehlende UE 5.6 und einen Linux-Host.

| Prüfung | Status |
| --- | --- |
| Python-Regressionen | Bestanden: 654 Tests |
| Portable C++-Produktionsregeln mit g++/UBSan | Bestanden: 450.630 Prüfungen in 13 Programmen |
| Katalogerzeugung, Kanten, String-/Unicode-Ausgabe | Bestanden: Katalog aktuell; 25 Graphen |
| Originale Rig-/Animations-/Prop-/Audioquellen | Bestanden: vorhandene binäre Quelldaten geprüft |
| Unreal Header Tool / natives UE-C++ | Blockiert: Engine fehlt |
| Native Askir-Automation | 68 Tests registriert, nicht ausgeführt |
| E-Interaktion, Dialogbuttons, HUD und Kamera | Nicht ausgeführt |
| Rurik-Platzierung, reale Karten/Assets/Navigation | Blockiert: Engine fehlt |
| Win64-Paket / eigenständiger Spielstart | Blockiert: Engine und Windows-Host fehlen |
| Vollständiges Durchspielen / Performance | Nicht ausgeführt; keine FPS-Messung |

Native neue Tests prüfen Graphen und Kontext, Legacy-Nodes, echte Torben-
Transaktionen, fehlgeschlagene Übergaben, Callback-Reentranz und Actor-Zerstörung.
Ein zusätzlicher vollständiger Snapshot-/SaveGameToMemory-/Restore-Test prüft
Ruriks gelernte Brunnenkunde, veränderte Begrüßung und abgelaufene Antwortrevision.
Ein weiterer Test stellt den Rückkehrzustand mit leerem Questdaten-Cache wieder
her und prüft erneute Datenauflösung sowie einmalige Auszahlung.
Diese nativen Tests sind erstellt und registriert; hier wurden sie nicht ausgeführt.

## Noch erforderliche Engine-Prüfung

1. Den V83-Buildrunner auf Windows mit UE 5.6 ausführen. Editor- und Win64-Build,
   beide erhaltenen Karten und der echte Rurik-Nachschritt müssen erfolgreich sein.
2. Torben über E ansprechen, Antworten vor/zurück wechseln und wahlweise Hilfe
   oder fairen Handel annehmen. Genau eine Questannahme und einmalige +2-Fraktion
   prüfen. Nicht angenommene Gespräche dürfen keine Belohnungen gewähren.
3. Zwei Banditen bezwingen, Fracht sichern und zurückkehren. Dialog muss zum
   jeweiligen Fortschritt passen; Übergabe muss genau 16 Gold, 100 XP und
   `item_002` gewähren. Erneute Gespräche dürfen die Belohnung nicht wiederholen.
4. Mit vollem Inventar zurückgeben: Fehler anzeigen, Gespräch/Quest behalten;
   Platz schaffen und danach erfolgreich zurückgeben.
5. Rurik ansprechen, Brunnenkunde lernen, speichern, Anwendung beenden und laden.
   Die neue Begrüßung muss erscheinen. Cursor und vor dem Laden erzeugte Buttons
   dürfen nicht fortgesetzt werden; neue Gespräche müssen wieder möglich sein.
6. Reichweite verlassen, Sichtlinie blockieren, Gespräch schließen, sterben und
   Welt wechseln. Keine verwaisten Panels, Cursor oder Eingabesperren akzeptieren.
7. Mara-Mission mit drei Scherben, zwei Wächtern und Kellertruhe vollständig
   spielen; 90 Gold/180 XP und die Speicherung weiterhin prüfen.
8. Hauptmenü, Einstellungen, Zielerfassung, Händler und Crafting vor und nach
   Dialogen benutzen. Darstellung/Schrift/Navigation manuell prüfen und auf
   dokumentierter Windows-Hardware Bildrate, Speicher und Ladezeiten messen.

Alle manuellen Punkte bleiben bis zu einem protokollierten Engine-Lauf
**Nicht ausgeführt**. Das Entwicklungsarchiv ist kein paketiertes Windows-Spiel.
