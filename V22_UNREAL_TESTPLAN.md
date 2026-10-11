# V22 Unreal Engine 5.6 – manueller Testplan
1. Projekt mit UE 5.6 öffnen, C++ kompilieren, Fehler dokumentieren.
2. QuestData-Assets `Q_HolzSammeln` und `Q_Lieferung` erstellen; verschiedene IDs und Ziel-IDs setzen.
3. Zwei Questgeber-Actors mit je einem QuestData-Asset in die Karte setzen; Mesh und Kollision zuweisen.
4. NavMesh und Interaktions-Trace im bestehenden Testlevel prüfen.
5. Beide Quests nacheinander annehmen; Inventar-Pickups einsammeln; beide Queststände separat kontrollieren.
6. Unbekannte ItemID, doppelte IDs, negative Zielmenge, negative Pickup-Menge testen.
7. Quest abschließen und sicherstellen, dass die zweite Quest unverändert bleibt.
8. Spiel speichern, schließen, laden und beide Fortschrittsstände vergleichen.
9. Ältere Spielstände testen; Migration alter aktiver Questziele ist noch offen.
10. Im Packaging-Build erneut testen; dies ist noch nicht erfolgt.
