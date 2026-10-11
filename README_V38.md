# Askir RPG V38

V38 erweitert V37 um einen persistenten Reisesnapshot. Vor `OpenLevel` speichert das Portal den Spielstand; nach dem Laden stellt der GameMode Inventar, Quests, Fortschritt und weitere im bestehenden SaveGame abgebildete Daten wieder her, ohne die Zielkarten-Ankunftsposition zu überschreiben. Loot-Historien früherer Karten werden beim Speichern desselben Slots erhalten.

**Nicht UE-kompiliert oder spielgetestet.** Die Snapshot-Rückkehr bei Fehlern, Level-Cooking, echte `.umap`-Maps, persistente Ausrüstung und alle weiteren offenen Punkte müssen folgen. Siehe `RELEASE_V38_50_SCHRITTE.md`.
