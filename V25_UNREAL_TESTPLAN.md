# V25 – Test im Unreal Editor 5.6
1. Projekt kompilieren. Compile-Fehler vor dem Test beheben.
2. Level mit Boden, PlayerStart, NavMeshBoundsVolume und NavMesh öffnen.
3. Enemy Blueprint von `ACharacter` ableiten; `UAskirCombatComponent` hinzufügen. Eine passende AIControllerClass konfigurieren.
4. `AAskirEncounterZone` ins Level ziehen und `EncounterID` setzen.
5. `AAskirEncounterSpawner` platzieren, `Zone`, `EnemyClass`, `EnemiesPerWave=2`, `WaveCount=2` setzen.
6. Play-in-Editor: Spieler betritt Zone; zwei Gegner sollten erscheinen. Prüfen, ob KI sie steuert.
7. Beide Gegner durch Kampfschaden töten; zweite Welle prüfen.
8. Nach letzter Welle `OnEncounterFinished` und Questziel-Update prüfen.
9. Negativtests: kein EnemyClass, blockierte Spawnposition, Gegner ohne CombatComponent, wiederholtes Betreten, ungültige QuestID.
10. Bei Bedarf zusätzliche Blueprint-Animationen, Effekte, Sound und UI binden. Diese Assets sind nicht im Code erzeugt.
