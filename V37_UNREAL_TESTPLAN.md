# V37 Unreal-Editor-Prüfung (noch nicht durchgeführt)

1. V37 in Unreal Engine 5.6 öffnen und C++-Modul kompilieren. `AAskirWorldPortal` und `UAskirWorldTravelSubsystem` müssen verfügbar sein.
2. In `Tools/generate_30_world_blockouts_v36.py` `DRY_RUN=False` setzen und **nur in einer gesicherten Kopie** ausführen. Bestehende Maps werden übersprungen.
3. Den INI-Ausschnitt `Config/WorldMapsToCook_V37.ini.txt` nach Prüfung in die Projekt-Packaging-Settings übernehmen.
4. GameMode `AAskirGameMode` und Game-Character für alle Story-Maps konfigurieren. L01 starten, Portal nach L02 betreten, Position neben Rückportal kontrollieren, nach L01 zurückkehren.
5. Prüfen: NPC aktiviert Portal nicht; zweimalige Überlappung verursacht keinen doppelten OpenLevel; fehlende Map wird als Fehler erkannt (derzeit keine Asset-Existenzprüfung im C++-Portal).
6. Inventar/Quests vor Reise ändern und nach Reise prüfen: **derzeit bekanntermaßen nicht persistiert**; hierfür später GameInstance/SaveGame-Integration entwickeln.
7. Map-Checks, Collision, Navigation, Quest-Actors, Cook, Windows-Build und tatsächliche FPS prüfen. Noch keine Ergebnisse vorhanden.
