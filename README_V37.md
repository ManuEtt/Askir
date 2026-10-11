# Askir RPG V37 – Weltreisen (801–850)

Neue C++-Klassen: `AAskirWorldPortal` (Spieler-Overlap, OpenLevel), `UAskirWorldTravelSubsystem` (Ankunftsstatus über Levelwechsel). Der GameMode setzt den Spieler beim Wechsel neben das Rückportal und erzeugt in Lxx-Weltkarten keine Demoblöcke. Der V36-Editor-Generator erzeugt optional funktionale Portale, wenn das C++-Modul zuvor kompiliert wurde. 30 Cook-Map-Einträge liegen als **manuell zu integrierender** INI-Ausschnitt bei. `Tools/validate_world_v37.py` prüft Weltdaten und Cook-Liste.

**Grenzen:** OpenLevel erhält den transienten Spieler-/Quest-/Inventarzustand nicht automatisch. Es fehlen eine echte globale Spielzustandspersistenz über Reisen, Save/Load aller Karten, Freischaltungen, 30 generierte `.umap`-Karten und UE-Kompilierung. Die Karten werden nur nach manuellem Setzen von `DRY_RUN=False` im Unreal Editor erzeugt. Keine Spielbarkeit behauptet.
