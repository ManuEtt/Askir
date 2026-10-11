# V16 – Save/Load-Integration
GameMode SaveGame/LoadGame übernimmt Weltzeit, Fraktionsruf, Dialogflags und einmalige Weltereignisse. Die erste `AAskirVillageDirector`-Instanz im Level wird als Weltuhr verwendet. Nach Laden werden Bewohner-Zeitpläne neu ausgewertet.
**Offen:** Keine persistente Speicherung der einzelnen NPC-Positionen oder Händlerbestände; für mehrere Dörfer ist ein eindeutiges Director-ID-System erforderlich.
**Hinweis:** SaveGame-Felder wurden erweitert; alte Speicherstände erhalten Defaultwerte.
