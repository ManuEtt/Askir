# Askir V67 — persistente Bossarenen und Questfortschritt

## Implementierte Änderungen
- Eine EncounterZone wird nach Laden eines als abgeschlossen gespeicherten Encounters nicht erneut aktiviert.
- Die Zone hat die neue Option `bAwardObjectiveOnEntry` (Standard `true` für Bestandskompatibilität). Für Kampfarenen **false** einstellen.
- Der Spawner lehnt Encounter-IDs ab, die nicht zur zugewiesenen Zone passen.
- Bei `bAwardObjectiveOnEntry=false` vergibt der Spawner Questfortschritt nach Sieg; falls eigene Completion-IDs fehlen, werden QuestID/ObjectiveID der Zone verwendet.
- Abgeschlossene Encounter werden weiterhin im SaveGame über `CompletedEncounters` gespeichert.

## Unreal-Editor-Einrichtung
Für jede Bossarena: Zone.EncounterID eindeutig setzen; Zone.bAwardObjectiveOnEntry=false; Zone.QuestID/ObjectiveID zuordnen; Spawner.Zone zuweisen; EnemyClass konfigurieren. Blueprint-/Level-Assets werden dadurch nicht automatisch erzeugt.

## Grenzen
Keine Unreal Engine 5.6 Kompilierung und kein PIE-Test. Die 50 nummerierten Themen sind dokumentierte Testfälle, keine 50 neuen Mechaniken.
