# V23 Testplan (Unreal Engine 5.6)
1. Projekt kompilieren, UnrealHeaderTool-Fehler zuerst beheben.
2. Spieler-Blueprint um Quest-, Trade-, Progression- und Inventar-Komponenten ergänzen.
3. Quest-Datenasset mit QuestID, mindestens einem Ziel und Gold/XP/Item-Belohnung anlegen.
4. Questgeber platzieren, Quest zuweisen, starten.
5. QuestTrigger platzieren und passende QuestID/ObjectiveID setzen; Spieler hindurchbewegen.
6. Beim Questgeber abgeben; Gold, XP, Item und Completed-Status prüfen.
7. Erneute Abgabe versuchen: keine zweite Belohnung.
8. Negative Belohnungen und fehlende Komponenten testen.
9. Händler-Actor platzieren und OnMerchantOpened im Blueprint mit einem eigenen UI verbinden.
10. Save/Load einschließlich ClaimedRewards überprüfen: GameMode enthält nun die Felder `SavedClaimedRewards` und `SavedPlayerGold`; im Editor die tatsächliche Persistenz und ältere Spielstände testen.
11. NavMesh, Kollision, Interaktionsdistanz, World Partition und Packaging testen.

Nicht als bestanden markieren, bevor diese Schritte in UE 5.6 tatsächlich ausgeführt wurden.
