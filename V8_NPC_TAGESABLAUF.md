# Version V8 – NPC-Tagesabläufe
Neue ActorComponent `UAskirDailyRoutineComponent`: Tagesstunden (0–23), sortierungsunabhängige Zeitpläne, zyklische Tagesgrenzen, Dorf- und Wachvorlagen, Aktivitätswechsel-Event und Destination-Tags.
**Editor-Schritte:** Komponente zum NPC-Blueprint hinzufügen; bei Weltzeit-Änderung `EvaluateAtHour` aufrufen; `OnRoutineChanged` mit Navigation zum passenden Ziel-Actor verbinden.
**Grenze:** Keine automatische Weltuhr oder NavMesh-Integration; die Komponente entscheidet Aktivitäten, bewegt NPCs nicht selbst.
