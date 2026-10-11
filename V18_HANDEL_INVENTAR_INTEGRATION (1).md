# Askir V18 – Händler mit echtem Spielerinventar verbinden

Neue `UAskirTradeBridgeComponent` verbindet den bisherigen Händlerbestand aus V11 mit `UAskirInventoryComponent` und dem Goldwert des Spielers. Die ItemID wird über den `Catalog` einem `UAskirItemData` zugeordnet.

## Neu
- Kauf prüft Gold und Händlerbestand und fügt anschließend die Ware dem Spielerinventar hinzu.
- Verkauf prüft zuerst den Besitz des Artikels und entfernt ihn vor der Gutschrift.
- Fehlschläge beim Item-Transfer werden nach Möglichkeit durch Rückabwicklung kompensiert.
- Ungültige Mengen, fehlende Items und Überläufe beim Verkauf werden abgewiesen.

## Im Unreal Editor
1. `UAskirTradeBridgeComponent` einem Shop-/Händler-Actor hinzufügen.
2. `Catalog` mit den ItemIDs aus dem Händlerbestand und passenden `UAskirItemData`-Assets befüllen.
3. Für `PlayerInventory` die Inventarkomponente des Spielercharakters übergeben.
4. Den Goldwert des Spielers als `int32&` aus einem Blueprint/C++-Aufrufer übergeben; bei Blueprints einen persistenten Spieler-Goldwert verwenden.
5. Buttons des Shop-Widgets an `BuyItem` und `SellItem` anbinden.

## Bekannte Grenzen
- Für Mehrspielerbetrieb ist eine serverautoritativ ausgeführte Transaktion nötig.
- Das ältere Händler-API `BuyFromMerchant`/`SellToMerchant` bleibt separat aufrufbar und kann bei falscher Verwendung weiterhin Items ohne Inventartransfer handeln; UI sollte ausschließlich die Bridge nutzen.
- Keine Unreal-5.6-Kompilierung oder Laufzeittests in dieser Umgebung.
- Das ältere `BuyFromMerchant` prüft noch keinen Gold-Overflow des Händlers. Dieser sollte vor einem Produktiveinsatz behoben werden.
