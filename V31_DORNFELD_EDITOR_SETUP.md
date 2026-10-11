# V31 – Dornfeld Vertical Slice: Unreal-Editor-Einrichtung

## Voraussetzungen
1. UE 5.6 Projekt kompilieren (hier nicht durchgeführt). Auf dem Spieler ist `DornfeldMission` automatisch vorhanden.
2. In einer Levelkarte `BP_MaraEvent` von `AAskirDornfeldEventActor` erstellen und `Event=ElderDialogue` setzen.
3. Drei Scherben-Actors anlegen: `Event=Shard`, `PersistentEventID=SHARD_01`, `SHARD_02`, `SHARD_03`.
4. Zwei Wächter-Actors als `Event=GuardianDefeated` mit `GUARD_01`, `GUARD_02`. **Nicht** interaktiv auslösen: Vom `OnDeath`-Event des jeweiligen Feindes `TriggerEvent(Player)` aufrufen. Die Platzierung des Event-Actors muss für die Distanzprüfung am Feind liegen.
5. Eine Truhe als `Event=ChestOpened`, `PersistentEventID=CHEST_01`. `TriggerEvent(Player)` **erst nach erfolgreichem Öffnen** der bestehenden `AAskirLootChest` aufrufen; nicht vor dem tatsächlichen Loot-Transfer.
6. `OnStageChanged` an das Quest-HUD binden; Text via `GetObjectiveText()` anzeigen. Stage und IDs sind im Player-Component gespeichert.
7. Mit dem bestehenden Save/Load-Slot speichern; `SavedDornfeldMission` wird serialisiert. Danach Editor neu starten und Spielstand laden.

## Manuelle Tests (offen)
- Vor Gespräch keine Scherben anrechenbar; nach Gespräch drei verschiedene IDs erforderlich.
- Doppelte ID erhöht den Zähler nicht; Wächter-Event funktioniert erst nach drei Scherben.
- `GuardianDefeated` darf nur über tatsächliches OnDeath signalisiert werden.
- Nach zwei Wächtern Truhe öffnen; Mara schließt Mission mit 90 Gold + 180 XP ab.
- Zweites Gespräch vergibt nichts; Save/Load in jeder Stufe und nach Abschluss prüfen.
- Distanzprüfung, fehlende IDs, fehlende Komponenten und Save-Korruption prüfen.

## Einschränkungen
Diese Implementierung stellt **keine** .umap, .uasset, UMG-Widgets, Animationen oder automatisch verbundene Feind-Blueprints bereit. Die neue Mission ist parallel zum bisherigen generischen Quest-System; gemeinsame Objective-IDs und Item-Belohnungen müssen noch angebunden werden. SaveGame enthält die Missionsstufe, nicht die Transform-Positionen dynamisch gespawnter Gegner. Ein echter UE-Compile- und PIE-Test steht aus.
