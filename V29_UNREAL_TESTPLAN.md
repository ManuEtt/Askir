# V29 – Unreal-Editor-Testplan

1. In Unreal Engine 5.6 Development Editor für Win64 kompilieren (hier nicht ausgeführt).
2. Zwei `UAskirItemData`-Assets für Brunnenscherbe und Amulett anlegen.
3. In einer Testmap `AAskirLootPickup` dreimal platzieren; Item, Quantity=1, QuestID und ObjectiveID setzen.
4. `AAskirLootChest` platzieren und Contents konfigurieren.
5. Interaktions-LineTrace muss `IAskirInteractable::Execute_Interact` mit Spieler als Interactor aufrufen.
6. Quest über Questgeber starten und drei Scherben einsammeln; Inventar- und Questzähler prüfen.
7. Aufnahme aus mehr als 225 cm Entfernung verweigern.
8. Doppelte Interaktion darf weder Items noch Questfortschritt verdoppeln.
9. Ungültiges Item bzw. ungültige Menge muss ohne Zustandsänderung fehlschlagen.
10. Truhe darf nur einmal Items ausgeben; invaliden Eintrag ablehnen.
11. Questgeber, Gegner-Encounter und Belohnungen integrieren; Spielstand laden/speichern und Konsistenz prüfen.
12. Editor-Assets, Material, Kollision, Animationsmontagen, Sound und UMG manuell oder über Editor-Scripting erstellen.

**Offen:** Chest- und Pickup-Zustände sind noch nicht persistent. Eine neue Spielsitzung setzt platzierte Actors zurück. Ohne echte `.umap`- und `.uasset`-Assets ist die Mission noch nicht als fertiges Level spielbar.
