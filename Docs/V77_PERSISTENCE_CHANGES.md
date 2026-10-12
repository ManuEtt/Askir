# V77 – Persistenz und Reisen

## Tatsächlicher Stand

Die nachfolgenden Änderungen sind implementierter C++-Quellcode. Unreal Engine
5.6 und der Windows-Toolchain fehlen in dieser Ausführungsumgebung. Kompilierung,
Unreal-Automation, reale Save-Dateien, Map-Ankunft und Windows-Laufzeittests sind
deshalb **Blockiert**. Python-Quellvertragstests ersetzen diese Tests nicht.

## Reparaturen

- Save-Format 3 erfasst die zuvor ausgelassenen Spielerwerte für Survival,
  Stealth, Morale, Travel, Camp, Companion, Exploration, Crafting, freigeschaltete
  Rezepte, Needs, Guard-Ausdauer sowie optionale Faction/Relationship-Komponenten.
  Trade-Favor und Trade-Volume werden zusätzlich zu Gold gespeichert.
- Der vollständige Map-Package-Pfad wird gespeichert. Ein positionswiederherstellendes
  Laden aus einer anderen Karte öffnet die gespeicherte Karte. Der Rückgabewert
  bedeutet in diesem Fall „Ladeanforderung angenommen“; Restore erfolgt nach Spawn.
- Alle strukturellen Werte, Versionen, endliche Zahlen, Questziele, v3-Missionsbelege
  und Loot-IDs werden vor Mutation geprüft. Item-Assets und Stack-/Slotgrenzen werden
  vor Restore aufgelöst/geprüft. Fehlende Assets verändern kein laufendes Inventar.
- V3-Dornfeld-Snapshots verlangen passende Stage, drei eindeutige Scherben, zwei
  Wächter, Truhenbeleg und denselben Reward-Beleg im Questjournal. Alte v1/v2-Saves
  werden über die bestehende Mission-/Questnormalisierung migriert.
- BankA/BankB speichern abwechselnd Generationen. Ein Write überschreibt nur die
  inaktive Bank und wird durch Rücklesen geprüft. Kaputte/ungültige Banken werden
  beim Laden übersprungen; die vorherige gültige Generation bleibt verfügbar.
  Alte Einzeldatei-Slots werden weiter gelesen. Das ist keine Garantie gegen
  gleichzeitigen Verlust beider Dateien, Hardwaredefekte oder Betriebssystem-Caches.
- Map-Loot gehört zur aktuellen GameInstance-Timeline. Speichern kopiert keine
  Zustände aus einem anderen Save-Slot. Laden eines älteren Saves ersetzt das
  Ledger und setzt vorhandene Pickups/Truhen auch wieder auf nicht konsumiert.
- OpenLevel erhält einen validierten Snapshot im GameInstance-Subsystem. Reisen
  verändert keine manuellen Save-Slots und benötigt keine Snapshot-Datei.
- Portal-Ankünfte prüfen freie Capsule-Positionen außerhalb sämtlicher Portalboxen,
  respektieren Portal-IDs und lokale Offsets und besitzen zwei Sekunden Reise-Sperre.
  Ohne Gegentor wird ein gültiger PlayerStart verwendet. Fehlende sichere Ankunft
  wird protokolliert; der Snapshot bleibt erhalten. Engine-Reisefehler räumen Pending auf.
- Clock-Restore setzt tägliche und einmalige Event-Belege ohne CheckEvents zurück.
  Der vorige SetClock-Aufruf konnte bereits vor Ledger-Restore One-Shots erneut auslösen.
- Restore replayt keine Demo-Todesereignisse. Der Demo-Ausgang zahlt XP nur einmal.
  Die alte Arena startet nur bei explizitem bEnableLegacyDemo. Runtime-Cube-Laden
  verwendet LoadObject statt des außerhalb von Konstruktoren unzulässigen FObjectFinder.
- Quest und Mission erhalten eine stille Installationsphase vor HUD-Benachrichtigungen.
  Mission-Relays gleichen Wächter und Questloot nach dem Restore ab. Charakter-Restore
  löscht laufende Angriffe und stellt Bewegung/Kollision entsprechend Health wieder her.

## Tests und Grenzen

Native Automation ist hinzugefügt: `Askir.Persistence.PayloadValidation`,
`SystemSnapshotRoundTrip`, `SaveBanksRecoverPreviousGeneration`,
`ClockRestoreDoesNotReplayEvents`. **Status: Blockiert**, nicht ausgeführt.
Die Banktests verwenden eindeutige temporäre Slots und räumen ihre Dateien auf.

Ein Snapshot ist eine validierte Wiederherstellung der erfassten Systeme; beliebige
Blueprint-Callbacks sind keine allgemeine Datenbanktransaktion. Externe Subscriber
müssen weiterhin eigene Transaktionen absichern. Nicht erfasst sind beliebige NPC-
Transform-/AI-Zustände, Händlerbestände, fremde Blueprint-Komponenten, noch nicht
implementierte Mana/Zaubereffekte und global stabile Beziehungen ohne stabile EntityIDs.
Pro Map wird der erste VillageDirector-Clock persistiert. Die bestehenden Weltkarten
verwenden eindeutige kurze Map-Namen als Loot-Schlüssel; gleichnamige Karten in mehreren
Ordnern sind damit nicht unterstützt. Diese Einschränkungen verhindern die Behauptung,
das vollständige RPG oder alle Weltzustände seien bereits fertig oder getestet.
