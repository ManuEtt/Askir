# V77 – Das Flüstern im Brunnen

Status: **Laufzeitquellcode implementiert; Unreal-Kompilierung, Editor-, Karten- und Windows-Spieltest blockiert, weil Unreal Engine 5.6 in dieser Umgebung fehlt.** Die Mission wird nicht als fertig spielbar oder abgenommen bezeichnet.

## Tatsächliche Änderungen

- Mara nimmt die Mission an; erst danach lassen sich die drei unterschiedlichen Scherben konsumieren. Reale `AAskirLootPickup` übertragen ihren Gegenstand ins Inventar und tragen `DornfeldShardID`. Leere und wiederholte IDs zählen nicht.
- Erst drei Scherben aktivieren die beiden Wächter. `AAskirDornfeldMissionRelay` bindet echte Combat-Todesereignisse und verwendet die zentrale zeitlich begrenzte `IsKillAttributedTo`-Zuordnung. Ein Umwelttod oder fremder Verursacher zählt nicht; ein nicht angerechneter Wächter bleibt für die Mission verfügbar.
- Die Kellertruhe trägt `DornfeldChestID` und öffnet erst nach zwei angerechneten Wächtern. Ihr gesamter Inhalt wird mit einer Inventartransaktion übertragen. Ein zu früher oder fehlgeschlagener Versuch konsumiert weder Truhe noch Inhalt.
- Mara zahlt bei der Abgabe genau **90 Gold und 180 XP**. Der persistente Beleg und der Wiedereintrittsschutz verhindern eine zweite Auszahlung. Bereits bezahlte Missionen bleiben beim Laden abgeschlossen.
- Der Dornfeldzustand synchronisiert `DORNFELD_BRUNNEN` im tatsächlichen Questjournal, einschließlich aller fünf Ziele und des Belohnungsbelegs. Missionsobjekte verwenden keine zusätzlichen generischen ObjectiveIDs, damit dieselbe Aktion nicht doppelt zählt.
- Wiederherstellung dedupliziert IDs in stabiler Reihenfolge, entfernt leere IDs und verfrühte Folgeschritte und leitet die zulässige Phase aus den vorhandenen Belegen ab. Stille Wiederherstellung erlaubt dem Save-System, zuerst den gesamten Snapshot zu installieren und anschließend einmal UI-/Weltbeobachter zu informieren.
- Relays stellen konsumierte Scherben, Truhen und Wächter aus dem Missionszustand wieder her. Die platzierten Wächter bleiben als Actors erhalten, damit auch das Laden eines früheren Spielstands funktioniert. NPC-Restore spielt keine Todes- oder Lootbelohnung erneut ab.
- Generische Questbelohnungen besitzen ebenfalls einen Transaktionsschutz gegen erneut eintretende Inventar-/Questdelegates. Gegenstände, Questabschluss und Belohnungsbeleg werden vor der ersten Benachrichtigung gemeinsam gesetzt, damit auch ein Speichern aus einer UI-/XP-Benachrichtigung keinen halben Belohnungsvorgang festhält. Einmalige Questtrigger benötigen eine stabile ID und beanspruchen den Beleg vor den Fortschrittsdelegates.

## APIs für den Kartenaufbau

`AAskirDornfeldEventActor` stellt Mara mit `Event=ElderDialogue`, `Visual` und `InteractionRange` bereit. Die Benutzeroberfläche zeigt Annahme/Abgabe als bestätigbare Unterhaltung. Für die Scherben werden tatsächliche LootPickups mit Item, Quantity=1, `PersistentLootID` und `DornfeldShardID` verwendet; für die Truhe `PersistentLootID` und `DornfeldChestID`.

Jeder der drei Pickups, der beiden Wächter und der Truhe hat einen Relay mit genau einer Quelle und einem stabilen `EventID`. Der lokale Spieler wird beim Binden aufgelöst. Der V77-Generator verwendet `Dornfeld_Shard_North`, `Dornfeld_Shard_East`, `Dornfeld_Shard_Well`, `Dornfeld_Guardian_1`, `Dornfeld_Guardian_2`, `Dornfeld_CellarChest`. Wächter-KillExperience=0 hält die Gesamtbelohnung dieser Sequenz bei 180 XP.

Neue/erweiterte APIs: `CanCollectShard`, `CanDefeatGuardian`, `CanOpenMissionChest`, `RestoreMission(Snapshot, bNotify)`, `NotifyMissionRestored`, `RestoreQuestProgress`, `NormalizeRestoredState(bNotify)` und `Relay.RefreshMissionState`.

## Nachweis und Grenzen

`Source/Askir/Mission/AskirDornfeldRules.h` enthält die tatsächlich vom Unreal-Component verwendeten, engineunabhängigen Phasen-, Zugangs- und Belohnungsregeln. `Tests/cpp/test_dornfeld_rules_v77.cpp` wurde mit g++ als C++17 einschließlich `-Wall -Wextra -Werror -pedantic` kompiliert und ausgeführt: **73 Prüfungen bestanden, 0 fehlgeschlagen**. Diese Prüfungen erfassen unter anderem alle Phasengates, doppelte/fehlende IDs, falsche Fortschrittsbelege, Reward-Wiederholung und Goldüberlauf. Sie ersetzen keinen Unreal-Test.

Zusätzliche Unreal Automation Tests prüfen reale Loottransfers, Combat-/Relay-Ereignisse, SaveGame-Speicherserialisierung, Missionserhalt und idempotente Belohnungen. Ihr Status ist **nicht ausgeführt / durch fehlende Engine blockiert**. Die Fixture-/Integrationstests werden unter `Askir.Mission.Dornfeld.*` registriert.

Der Editor-Kartengenerator ist vorhanden und weist native Actors, Gegenstände und Relays zu. Eine echte `.umap`, sichtbare Charakteranimationen, Navigation, UMG-Verhalten im laufenden Spiel und ein durchgespielter Dorfablauf sind erst nach Engine-Generierung, Kompilierung und Test nachgewiesen. Die gesamte Kampagne, 30 spielbare Levels und ein Windows-Build bleiben offen.
