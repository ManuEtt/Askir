# Askir RPG V35 – Integrationsstabilität

Ausgangspunkt: V34. Implementiert: automatische Bind-Wiederholungen für Dornfeld-Missions-Relays, Verhinderung interaktiv gefälschter Wächter-/Truhenabschlüsse, manuelle HUD-Erstsynchronisierung über `RefreshObjective()`.

## Einrichtung

Platziere die sechs `AAskirDornfeldMissionRelay`-Instanzen mit eindeutigen `EventID`-Werten und genau einer Quelle pro Relay. Weise `MissionPlayer` zu oder lasse ihn bei Spielstart automatisch ermitteln. Bei späterem Spawn wiederholt das Relay die Bindung alle 0,5 Sekunden (konfigurierbar).

Im UMG-Widget `OnObjectiveChanged` abonnieren, anschließend `RefreshObjective()` aufrufen, damit auch bei spät erzeugtem Widget sofort der aktuelle Text angezeigt wird.

**Sicherheit:** `AAskirDornfeldEventActor` darf die Guardian- und Chest-Ziele nicht mehr direkt per `TriggerEvent` abschließen. Für echte Loot-/Kampfereignisse `AAskirDornfeldMissionRelay` verwenden.

**Nicht ausgeführt:** Unreal-Kompilierung, Editor-Tests, Erstellung echter `.umap`- oder `.uasset`-Dateien und Windows-Paketierung.
