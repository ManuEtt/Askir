# Askir V70 — robuste Begegnungsbeute

## Implementiert
- Persistente Begegnungen verwenden für die gewichtete Loot-Auswahl einen stabilen Seed aus der Encounter-ID. Ein fehlgeschlagener Inventareintrag oder erneutes Laden ändert die Auswahl für diese ID nicht.
- Begegnungen ohne ID behalten eine frische Zufallsauswahl pro Sieg.
- Der Zufallswert wird explizit auf den gültigen Gewichtsbereich begrenzt.
- Persistente Begegnungen starten nicht ohne Quest-Komponente als Abschluss-Ledger.
- Nach dem Abschluss-Event wird die Referenz auf den aktiven Spieler freigegeben.

## Grenzen
- Identische Encounter-IDs geben bei identischen Loot-Tabellen dieselbe Belohnung für alle Spieler/Spielstände; ID muss pro persistentem Encounter eindeutig sein.
- Änderungen an einer Loot-Tabelle ändern möglicherweise das Ergebnis trotz gleichen Seeds.
- Es gibt weiterhin **keine transaktionale, crash-atomare Speicherung** von Inventar, Quest und Encounter-Ledger. Der bestehende Save-Pfad muss in Unreal end-to-end geprüft werden.
- Kein Unreal-Engine-Build oder PIE-Test hier; Tests sind statische Offline-Prüfungen.
- Die 50 Fälle sind dokumentierte Testaufträge, nicht 50 implementierte Funktionen.
