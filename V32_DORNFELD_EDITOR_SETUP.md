# Dornfeld: Mission-Relay in Unreal Engine 5.6 einrichten

1. Projekt kompilieren, `DornfeldMission` am Spieler prüfen.
2. Mara als `AAskirDornfeldEventActor` mit `ElderDialogue` platzieren.
3. Drei `AAskirLootPickup` mit gültigem `UAskirItemData` konfigurieren; je ein `AAskirDornfeldMissionRelay` mit `ShardPickup` und `EventID=Shard_01` bis `Shard_03` platzieren.
4. Zwei feindliche Actoren mit `UAskirCombatComponent` platzieren; je ein Relay mit `GuardianActor` und `EventID=Guardian_01` bzw. `Guardian_02` verbinden.
5. Eine `AAskirLootChest` konfigurieren und Relay mit `MissionChest` und `EventID=CellarChest_01` hinzufügen.
6. Jedes Relay: `MissionPlayer` optional dem Spieler zuweisen; genau eine Quelle konfigurieren.
7. Für Save/Load bei Pickups und Chest separate eindeutige `PersistentLootID` setzen; diese IDs müssen nicht mit den Relay-EventIDs identisch sein.
8. UI: `DornfeldMission.OnStageChanged` abonnieren, `GetObjectiveText()` anzeigen.
9. Prüfen: keine Scherbe ohne erfolgreichen Inventartransfer, keine Truhe ohne erfolgreichen Loot-Transfer, Wächter nur bei echtem Tod.
10. Automation-Test `Askir.Mission.Dornfeld.ProgressionAndDedup` über Session Frontend ausführen.

**Grenzen:** Dies sind Editor-Einrichtungsschritte; keine fertigen `.umap`/`.uasset`-Level im ZIP. Gegner-Kill-Zuordnung zum tatsächlichen Spieler ist noch nicht abgesichert (Welt-Tod löst Event aus). Die Event-Reihenfolge bleibt stufenbasiert.
