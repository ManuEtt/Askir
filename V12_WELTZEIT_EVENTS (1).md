# Version V12 – Weltzeit und Ereignisse
`UAskirWorldClockComponent` führt Spieltag/Stunde, Echtzeit-zu-Spielzeit, Pause, Zeitsprung, Stunden-Events, tägliche/einmalige Ereignisse und einen einfachen Tageslichtfaktor.
**Editor:** Komponente an GameState-/WorldManager-Actor hängen; `OnHourChanged` mit `EvaluateAtHour` der V8-NPC-Komponenten verbinden. `OnWorldEvent` für Markttage, Wachwechsel, Queststarts nutzen. Tageslichtfaktor kann Directional-Light-Intensität steuern.
**Grenze:** Kein echter dynamischer Sonnenstand oder Wetter. Clock-Zustand und einmalige Events müssen für Save/Load noch gespeichert werden. Tick läuft nur, wenn Owner aktiv ist.
