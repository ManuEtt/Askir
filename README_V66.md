# Askir RPG V66 — persistente Begegnungsabschlüsse

## Implementiert
- Persistente CompletedEncounters-Liste in der Questkomponente und im SaveGame.
- Speichern und Wiederherstellen durch AskirGameMode.
- Spawner verweigert den Neustart bereits abgeschlossener Begegnungen anhand der stabilen EncounterID.
- Questfortschritt wird nur bei erstmaligem Abschluss einer identifizierten Begegnung vergeben.
- Leere EncounterIDs bleiben als wiederholbare, nicht persistente Begegnungen behandelt.
- Alte Saves bleiben lesbar; fehlende Felder erhalten den Standardwert einer leeren Menge.

## Einschränkungen
- Unreal-5.6-C++-Build und PIE-Laufzeittest sind noch ausstehend.
- Level-Designer müssen stabile eindeutige EncounterIDs vergeben.
- Bestehende bereits abgeschlossene Begegnungen aus alten Saves können nicht rückwirkend erkannt werden.
- Questbelohnungen erfolgen weiterhin über das bestehende Quest-Turn-in-System; keine erfundenen Loot-Assets.
