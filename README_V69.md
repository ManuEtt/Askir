# Askir V69 — gewichtete Bossbeute

## Implementiert
- `FAskirEncounterLootEntry` mit Item, Menge und Gewicht.
- `CompletionLootTable` auf `AAskirEncounterSpawner`: eine gültige gewichtete Belohnung pro Sieg.
- Ungültige Items und Gewichte <= 0 werden ignoriert; bei leerer/ungültiger Tabelle greift die feste V68-Belohnung.
- Mengen werden auf 1–999 begrenzt und Gewichte als `int64` summiert.
- Doppelte persistente Begegnungen werden vor Vergabe abgewiesen.
- Fehlgeschlagenes `AddItem` verhindert die Registrierung als abgeschlossene Begegnung.

## Grenzen
- Kein Crash-atomarer Commit zwischen Inventar und Quest-Ledger: bei Absturz vor dem Speichern können Inkonsistenzen entstehen.
- Zufällige Auswahl ist nicht save-deterministisch. Belohnungen werden bei nicht persistenten Begegnungen erneut gezogen.
- Loot-Assets müssen im Unreal Editor konfiguriert werden; kein `.uasset`-Import oder UE-PIE-Test erfolgt.
- 50 Szenarien sind dokumentierte Prüfaufträge, keine 50 unabhängigen Spielfunktionen.
