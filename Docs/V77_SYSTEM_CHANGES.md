# V77: Inventar, Crafting, Handel und Build-Korrekturen

Diese Änderungen sind tatsächlich im C++-Quellcode integriert. Ein Unreal-Build, Editor-Start, Blueprint-Test und Windows-Build wurden in dieser Umgebung nicht ausgeführt: Unreal Engine 5.6, UnrealBuildTool und Windows-SDK stehen nicht zur Verfügung. Es wird keine Laufzeit-Spielbarkeit behauptet.

## Reflektion und Build

- Die beiden zuvor gleichnamigen `UAskirInteractionComponent`-Klassen hatten kollidierende UCLASS- und Generated-Header-Namen. Die NPC-Komponente heißt nun `UAskirNPCInteractionComponent`, ihre Dateien liegen unter `Source/Askir/NPC/AskirNPCInteractionComponent.*`. Die Trace-Komponente unter `Interaction/` behält ihren Namen.
- Die zwei verschiedenen reflektierten `FAskirScheduleEntry`-Strukturen sind jetzt eindeutig. Simulation verwendet `FAskirSimulationScheduleEntry`; die tägliche NPC-Routine verwendet weiterhin `FAskirScheduleEntry`.
- Jede reflektierte Variable in Needs, Simulation-Schedule, Relationship und Market besitzt eine eigene UPROPERTY-Deklaration. Mehrere C++-Variablen hinter einer UPROPERTY sind für UHT ungeeignet.
- Der Relationship-Map mit strukturiertem Schlüssel werden keine Blueprint-Map-Pins mehr erzeugt. Der native SaveGame-Map bleibt erhalten; Blueprint-Zugriff erfolgt über `GetRelationship`. EntityID erhält Identitäts-/Hash-Traits für reflektierte Map-Verwendung.
- `Askir.Build.cs` nimmt `UnrealEd` ausschließlich bei `Target.bBuildEditor` auf, damit die neue Level-Werkzeugbibliothek Editor-Funktionen verwenden kann.
- `PlayerGold`-Referenzen in Merchant und TradeBridge sind mit `UPARAM(ref)` versehen. Blueprint-Aufrufe benötigen den vorhandenen Goldwert als Eingabe und bekommen den veränderten Wert zurück.

## Inventar

`ExchangeItems(RemovedItems, AddedItems)` prüft und berechnet einen vollständigen neuen Inventarzustand, bevor er übernommen wird. Doppelte Zutaten werden aggregiert. Ausgaben können ihre eigenen fehlenden Eingangsanforderungen nicht nachträglich bezahlen. Bei jeder Ablehnung bleibt das Inventar unverändert; bei Erfolg wird genau eine Änderung gemeldet.

Stacks beachten `MaxStackSize`. Einzelgegenstände können insgesamt höchstens `MAX_int32` erreichen. `MaxInventorySlots` begrenzt die Zahl der Stacks auf maximal 4096 und verhindert unbeschränkte Speicherallokation aus fehlerhaften Mengen. Ungültige Gegenstände, nichtpositive Mengen und bereits inkonsistente Stacks werden abgelehnt. `HasItem` akzeptiert keine nichtpositiven Mengen. Consumables werden vor ihrer Wirkung entfernt.

Die Änderungsdelegates laufen unter einer Transaktionssperre. Rekursive Änderungen über die öffentlichen Inventarmethoden werden während der Meldung abgewiesen. Native C++-Aufrufer müssen weiterhin die Transaktionsmethoden benutzen; direkte Schreibzugriffe auf `Items` umgehen diese Absicherung.

## Handel

Die TradeBridge prüft Item-ID, Katalog-Eindeutigkeit, Bestände, Inventarkapazität, Gold und mögliche Überläufe. Danach werden Inventar, Spieler-Gold, Händler-Gold und Händlerbestand zusammen übernommen. Erst dann erhalten Beobachter eine Meldung. Die frühere preisabhängige Rückabwicklung nach bereits ausgelösten Meldungen entfällt. Rekursive Transaktionen desselben Händlers oder Inventars werden während der Meldung abgelehnt.

Preisberechnungen verwenden Double-Zwischenwerte; ein Verkauf nahe `MAX_int32` läuft nicht mehr über Float-Rundung außerhalb des Integerbereichs. Das einfachere Economy-Marktmodell rundet Gesamtpreise vor der Abbuchung auf volle Goldstücke auf: eine Ware für 0,01 Gold kostet mindestens 1 Gold. Fehlerhafte oder nichtendliche Preise und mehrdeutige Marktzeilen werden abgelehnt.

## Crafting

`FAskirCraftingRecipe` beschreibt reale Item-Zutaten und eine reale Item-Ausgabe. `CraftRecipe(Inventory, RecipeID, Quantity)` benötigt ein bekanntes Rezept, passende Fertigkeit, alle Zutaten und Platz für den vollständigen Ausgang. Mengenberechnungen verwenden geprüfte Integer-Produkte. Zutaten, Ausgabe und Crafting-Zähler werden atomar übernommen. Rezeptfreischaltungen stehen in `UnlockedRecipes`; diese Liste wird vom V77-Save-Pfad erfasst und wiederhergestellt.

Der vorhandene `CraftItem(Cost)`-Zählerpfad bleibt zur Kompatibilität bestehen. Er ist keine Inventar-Rezeptherstellung. Die neuen Rezeptdaten müssen im Editor oder in einer geprüften Content-Pipeline mit tatsächlichen Item-Assets konfiguriert werden. Diese Änderung allein liefert weder Crafting-UMG noch fertige Rezept-Assets.

## Weitere Zustandskorrekturen

Gold, Materialien, Handelsvolumen und Reputation verwenden geprüfte oder saturierende Arithmetik. Progression begrenzt Levelschwellen vor der Multiplikation und schützt Attributpunkte vor Überlauf. Weltzeit rechnet mit Int64 und normalisiert den Kalender. Needs lehnt negative oder nichtendliche Ruhezeiten ab; Relationship lehnt nichtendliche Änderungen ab.

## Nachweise

| Prüfung | Status | Tatsächlicher Umfang |
|---|---|---|
| Native C++-Regression CheckedMath | Bestanden | 511 Assertions am produktiv verwendeten Header; C++17, `-Wall -Wextra -Werror -fsanitize=undefined` |
| V17 Strukturvalidator | Bestanden | Umbenannte NPC-Dateien vorhanden, Generated-Includes geprüft; keine Engine-Kompilierung |
| `Askir.Inventory.AtomicExchange` | Blockiert | Unreal-Automation-Test implementiert, Engine nicht verfügbar |
| `Askir.Economy.AtomicInventoryTrade` | Blockiert | Unreal-Automation-Test implementiert, Engine nicht verfügbar |
| `Askir.Crafting.AtomicRecipe` | Blockiert | Unreal-Automation-Test implementiert, Engine nicht verfügbar |
| UHT, Unreal-C++-Build, Blueprint-Verifikation | Blockiert | UE5.6-Werkzeuge fehlen |
| Windows-Package und Start außerhalb Editor | Blockiert | Engine und Windows-Buildumgebung fehlen |

Portable Regression aus dem Projektverzeichnis:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=undefined Tests/cpp/test_checked_math_v77.cpp -o /tmp/askir_checked_math_regression
/tmp/askir_checked_math_regression
```

Die Unreal-Tests prüfen unter anderem aggregierte Zutaten, fehlende Ressourcen, Kapazitätsfehler ohne Teilverlust, Grenzen bei maximalen Itemmengen, fehlende Händlerliquidität, vollständige Gold-/Bestandsänderungen, Rezeptfreischaltungen, Fertigkeitsgrenzen und Batch-Überläufe. Diese Tests müssen nach erfolgreichem UE5.6-Build ausgeführt werden; bis dahin bleibt der Laufzeitstatus ungetestet.
