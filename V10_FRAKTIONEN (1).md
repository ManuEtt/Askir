# Version V10 – Fraktionen und Ruf
`UAskirReputationComponent` verwaltet Ruf pro Fraktion von -100 bis +100, Feindschafts-/Freundschaftsschwellen, Änderungsereignisse und einen Preisfaktor (0,75–1,25).
**Editor:** Dem Spieler die Komponente hinzufügen; Quests, Dialogantworten und Kämpfe rufen `ChangeReputation` auf. Gegner-KI kann `IsHostile` abfragen.
**Grenze:** Die Anbindung an bestehende KI und Händler ist noch nicht automatisch verdrahtet. Keine Speicherung der Rufwerte im vorhandenen SaveGame.
