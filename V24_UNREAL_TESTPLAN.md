# V24: Durchgängiger Dorf-Test (manuell in Unreal 5.6)

1. Projekt in UE 5.6 kompilieren. Bei Fehlern zuerst den Build-Log sichern.
2. Neues Level mit Boden, PlayerStart, NavMeshBoundsVolume und Licht anlegen.
3. Vorhandene V19-GLB/FBX-Prototypen über die dokumentierte Pipeline importieren.
4. Quest-Data-Asset mit `QuestID=VillageIntro`, ObjectiveID `EnterMarket`, RequiredCount=1 erstellen.
5. Questgeber aus V22 platzieren, Quest beim Spieler aktivieren.
6. `AAskirEncounterZone` im Markt platzieren: `EncounterID=MarketArrival`, `QuestID=VillageIntro`, `ObjectiveID=EnterMarket`, `bRequireActiveQuest=true`.
7. PIE starten: Ohne aktive Quest darf das Event nicht feuern; mit aktiver Quest einmal.
8. Über Blueprint `OnEncounterTriggered` an ein visuelles Hinweis-Widget oder Gegner-Spawn anschließen. Ohne diese Verbindung spawnt der Actor keine Gegner.
9. Händlerinteraktion und Questabgabe mit V23-Funktionen separat testen.
10. Speichern/Laden, NPC-NavMesh, Kollisionen, Animationen, Materialien und Packaging testen.

**Keine fertige `.umap`, kein automatischer Gegner-Spawn, keine bestätigte UE-Kompilierung.**
