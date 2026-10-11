# Unreal Engine 5.6 Testplan – V30

1. Visual Studio-Projektdateien aus `AskirRPG.uproject` erzeugen und Development Editor Win64 bauen.
2. Testmap `Dornfeld` anlegen; zwei `AAskirLootPickup` und eine `AAskirLootChest` platzieren.
3. Pro Actor einzigartige `PersistentLootID` eintragen; bei Truhe QuestID/ObjectiveID setzen.
4. Quest starten, zwei Pickups aufsammeln, Truhe öffnen; Inventar und Questfortschritt prüfen.
5. SaveGame auslösen; Level neu starten; LoadGame auslösen; Pickups und Truhe müssen verborgen bleiben.
6. Anderen Spielstand laden; nicht gesammelte Beute muss wieder erscheinen.
7. Prüfen: Doppelte IDs erzeugen Logwarnung; IDs über verschiedene Maps dürfen gleich sein.
8. Prüfen: Spawn-Pickups ohne ID sind absichtlich nicht persistent.
9. Negative Fälle: zu große Interaktionsdistanz, fehlendes Item, fehlendes Inventar.
10. Regression: XP, Gold, Questbelohnung, NPC-Dialog und Kampf testen.

**Offen:** In dieser Umgebung wurde kein Unreal-Compiler und kein Unreal-Editor ausgeführt.
