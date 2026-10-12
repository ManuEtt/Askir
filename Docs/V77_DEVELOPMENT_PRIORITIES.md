# ASKIR RPG – priorisierte Entwicklung nach dem V76-Audit

Stand: 12. Oktober 2026. Basis: `Docs/V77_TECHNICAL_AUDIT.md`, unverändertes V76-Archiv. Dies ist die Arbeitspriorisierung für konkrete Implementierung; sie ersetzt weder Spielinhalte noch abgeschlossene Tests. Die Engine-Abnahme bleibt blockiert, solange Unreal Engine 5.6 und die Windows-Build-/Testumgebung fehlen.

## P0 – Quellen, Startpfad und Datensicherheit

1. Vor Änderungen Original-ZIP sichern; SHA-256 und Ausgangsdateiliste festhalten. Lokale Git-Historie beginnen und generierte Dateien sinnvoll ignorieren.
2. Reflektierte Namenskollisionen `UAskirInteractionComponent` und `FAskirScheduleEntry` beseitigen. Jede UPROPERTY einzeln deklarieren; private Blueprint-Properties und Combat-API-Zugriff korrigieren.
3. Startkonfiguration auf existierende native Klassen umstellen. ConstructorHelpers aus Laufzeitfunktionen entfernen. Enhanced Input aktiv konfigurieren und reale Input-Actions/Mapping-Zuweisung herstellen.
4. E-Interaktion für `IAskirInteractable`, NPCs, Loot, Questobjekte und Mara vereinheitlichen. Reichweite, Sicht und toter Spieler müssen zu konsistenten Ergebnissen führen.
5. Eine Schadens-/Killzuordnung für Spieler, NPC, Owner-/Instigator-Projektile und Umweltschaden verwenden. Ablaufzeit, Tod einmalig, Restore ohne neue Belohnung und doppelte Death-Callbacks absichern.
6. Save-/Load-Mapwechsel, Arrival-Position und Character-Lifecycle wiederherstellen. Inventory-/Quest-/Gold-/Loot-Zustände vor Änderung validieren; Zahlengrenzen, Versionierung und Wiederaufnahme prüfen.

**Prüfgate:** Mit verfügbarer Engine UHT + Development Editor Build erfolgreich, native Klasse erreichbar, Start ohne Runtime-Fatal, reale Eingabe und Save/Load-Einzeltests bestanden. Offline-Tests dürfen dieses Gate nicht automatisch erfüllen.

## P1 – „Das Flüstern im Brunnen“ als erste vollständige Sequenz

1. Eine echte Dornfeld-Testkarte aus dem Editor erzeugen; Boden, Häuser, Brunnen/Keller, Licht, Spawn, NavMesh und Kollisionen speichern. Editor-Erfolg und `.umap`-Dateiexistenz separat prüfen.
2. Mara mit echter Mission-Interaktion verbinden. Drei unterschiedliche persistente Scherben, zwei reale Schattenwächter mit Kill-Relay und die persistente Kellertruhe platzieren. Vorzeitiger Verbrauch darf keine Mission blockieren.
3. Charakter-/NPC-Rohmodelle mit Blender riggen und als reale FBX-Dateien exportieren; Idle/Walk/Run/Jump/Sprint/Dodge/Attack/Block/Hit/Death/Cast-Clips herstellen. Importierte SkeletalMesh-/Skeleton-/Animation-Artefakte aus dem Editor prüfen.
4. Echtzeit-HP/Stamina/XP/Gold und Missionziel anzeigen. Interaktionsprompt, Pause, Save/Load und Tod/Wiederaufnahme mit echten Gameplaydaten verbinden.
5. Mara-Turn-in atomar: exakt einmal 90 Gold und 180 XP. Stage, drei Shard-IDs, zwei Guardian-IDs, Chest-ID und Reward-Claim persistent speichern.
6. Mission in einer Testwelt ausführen und zusätzlich manuell mit Eingaben durchspielen. An jedem Missionsabschnitt speichern/laden; frühzeitige Interaktion, falscher Killer, doppeltes Objekt, doppelter Turn-in und Wiederaufnahme nach Tod prüfen.

**Prüfgate:** Regulärer Start → Mara → drei Scherben → zwei Wächter → Truhe → Mara → Belohnung → Slot schreiben → Slot lesen. Nach erneuter Interaktion keine weitere Belohnung. Echter Level, echte Actors, native Integrationstests und manuelles Protokoll sind Pflicht.

## P2 – Vollständige Kernsysteme mit Laufzeitanbindung

| Schwerpunkt | Konkrete Implementierung | Abnahme |
|---|---|---|
| Inventar/Ausrüstung | Item-Datenassets aus 60 Datensätzen, Slots, echte Damage-/Armorwerte, Stacks, ItemUse, Tooltips | Ausrüsten/benutzen/verkaufen/speichern/laden; Stats und Bestände konsistent |
| Crafting | Rezeptregistry, Zutatenbestände, Stationen, Output, Unlocks, Transaktionsgrenzen | Erfolgreiches Rezept verbraucht exakt Zutaten; fehlende Bestände ändern nichts |
| Handel | Ein Wallet-/Stock-/Pricing-Pfad, Händlerdaten, Buy/Sell-UI, persistenter Stock, Reentrancy-Schutz | Buy/Sell/Doppelaufruf/Overflow/Save/Load geprüft |
| Magie | 24 datengesteuerte Effekte, Mana, Cooldowns, Target/Area/Projectile, Heal/Shield/Control, Anim/VFX/Audio/UI | Jeder Zauber einzeln mit Kosten, Cooldown, Wirkung und Restore überprüft |
| Kampf/KI | Waffentypen, Crit/Combo/Status, ein AI-Entscheider pro Pawn, Patrol/Ranged/Caster/Group/Leash | Spieler/NPC/Projectile/Environment-Kills und Kampfzustände mit Navigation überprüft |
| Dialog/Fraktionen | JSON-Nodes laden, Conditions/Quest-/Faction-/Relationship-Effekte, persistente Player-Choices | Choices ändern nachweislich Dialoge/Questzugang/Fraktion; Save/Load erhält sie |
| UMG | Menü/Pause/Settings/Stats/Questjournal/Inventory/Equipment/Magic/Crafting/Shop/Dialogue/Save/Ending | Widgetdaten und Aktionen mit realem Spielzustand getestet |

**Prüfgate:** Keine reine Datenliste zählt als System. Jedes System benötigt ausführbare Actors/Components, reale Datenbindung, UI und beobachtete Runtime-Tests.

## P3 – Zusammenhängende Kampagne und mindestens 30 Levels

1. Bestehende drei Kontinente, sechs Fraktionen, Städte und fünf Siegel als Kanon erhalten. Jeder Kapitelmission konkrete Start-, Ziel-, Konflikt-, Boss- und Abschlussbedingungen geben.
2. Campaign-Director mit Kapitel-/Missionzustand, Entscheidungen und drei Enden implementieren. Reise-/Fraktions-/NPC-Gates auf persistente Bedingungen beziehen.
3. 30 Manifestregionen zu echten Levels ausbauen: pro Region eigenes geografisches Layout, Gebäudefunktion, Route, Interior/Dungeon, Beleuchtung, Sound, NavMesh, NPCs, Gegner, Quests, Loot und Rückweg. JSON-Koordinaten und Würfelmarker allein bleiben unfertig.
4. Portale/Schiffsreisen/Schnellreise im Zusammenhang mit Storygates konfigurieren. Arrival-Spawns außerhalb von Triggern, regionale Loot-/Quest-/Encounter-Ledger und Map-Cook-Liste prüfen.
5. Mindestens 20 der vorhandenen Nebenquests mit echten Zielobjekten und NPCs autorieren. Generische `target_001`-Strings durch konkrete Actor-/Region-IDs ersetzen; Belohnungen und Conditions persistent machen.
6. Tagesabläufe und Wachen durch echte Residents/Workplaces/NavMesh betreiben. Spieluhr mit Sonne/Sky verbinden; Wetter, Audio, Begegnungen und Weltreaktionen implementieren.
7. Pro Level die Einzelmatrix aus `Reports/V77_inventory.json` nach echter Ausführung fortschreiben. Fehlende oder blockierte Tests dürfen nicht als bestanden überschrieben werden.

**Prüfgate:** Jede `.umap` erfolgreich geladen, betreten und gespielt; Kollision/NavMesh/NPC/Gegner/Quest/Travel/Save geprüft. Hauptgeschichte von regulärem Menüstart bis zu jedem der drei Enden ohne manuelles Kartenöffnen durchlaufen.

## P4 – Assets, Qualität, Performance und Win64-Lieferung

1. Modell-/Animations-/Material-/Texture-/VFX-/Audio-Produktion schließen. Nur eigene oder konkret lizenzierte Assets verwenden; Quelle, Rechte und Änderungen je externem Asset dokumentieren.
2. Blueprint-Kompilierung, DataValidation, MapCheck, ReferenceCheck, natives Automation und Campaign-Smoke-Test in reproduzierbare Build-Schritte aufnehmen. JSON-Berichte müssen tatsächliche ausgeführte Stufen wiedergeben.
3. Auf verfügbarer Zielhardware Frame-Time, 1080p-FPS, CPU-/GPU-Budget, Speicher und Ladezeiten messen. Daraufhin LOD/HLOD/Streaming, Materialkomplexität und AI-Tickbudgets anpassen.
4. Development-/Shipping-Win64-Build mit UAT erstellen, alle 30 Maps cooken und Paket außerhalb des Editors starten. Controls, Regionreise, komplette Dorfmission und Save/Load im Paket erneut prüfen.
5. Quellen-/Assetprojekt und Windows-Paket getrennt zippen; Checksummen, Startanleitung, Lizenzliste, Test-/Performanceberichte und konkrete Restblocker beilegen. GitHub nur bei vorhandenem tatsächlichem Schreibzugriff synchronisieren.

**Prüfgate:** Zwei überprüfbare Archive, erfolgreicher Buildlog und Paket-Testlog. Ohne Engine/Windows-Lauf entsteht lediglich ein Quellen-/Rohasset-ZIP; es wird nicht als spielbarer Windows-Build bezeichnet.

## Abhängigkeiten und eigenständig ausführbare Arbeit

Die Engine-Blockade verhindert Content-Serialisierung, UHT/UBT, PIE, Cooking und Runtime-Abnahme. Sie verhindert keine Quellreparaturen, Blender-Modelle/Rigs/Clips, Original-Audio/Texturen, Editor-Automation, saubere Transaktionslogik, Datenvalidierung, native Testquellen oder Git-Sicherung. Diese unabhängigen Arbeiten sollen weiterlaufen. Ein `.umap`-/`.uasset`-Dateiname ohne von Unreal erzeugtes gültiges Package erfüllt kein Gate.
