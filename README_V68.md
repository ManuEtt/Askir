# Askir V68 — Bossbelohnungen und Inventar

## Implementiert
- EncounterSpawner: optionale `CompletionRewardItem` und `CompletionRewardQuantity` (1–999).
- Bei erstmaligem persistentem Abschluss wird das Item in das Spielerinventar gelegt.
- Bei bereits registriertem Abschluss erfolgt keine doppelte Vergabe.
- Fehlt das Inventar bei konfigurierter Belohnung, bleibt die Begegnung wiederholbar und wird nicht als abgeschlossen markiert.
- Bestehende Quest- und SaveGame-Logik wird wiederverwendet; kein neues Save-Format nötig, da Inventar und CompletedEncounters bereits gespeichert werden.

## Grenzen
- Die 50 Themen sind Regression-Szenarien, keine 50 neuen Mechaniken.
- Die Belohnung muss im Unreal Editor als UAskirItemData-Asset konfiguriert werden.
- Es gibt noch keine Transaktions-/Crash-Recovery zwischen Encounter-Ledger und Inventar; UE-Kompilierung und PIE-Tests stehen aus.
