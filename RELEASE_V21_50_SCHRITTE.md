# Askir V21 – nächster 50-Schritte-Entwicklungsblock

**Status:** Quellcode implementiert, nur statisch geprüft. Nicht in Unreal Engine kompiliert oder gespielt.

- [x] 01. V20-Basis vollständig übernommen
- [x] 02. ZIP-Einträge sicher entpackt
- [x] 03. Projektstruktur unverändert erhalten
- [x] 04. Unreal-Modul-Abhängigkeiten geprüft
- [x] 05. C++-Quellordner vorhanden
- [x] 06. Zivilisten-AI-Controller angelegt
- [x] 07. Controller von AAIController abgeleitet
- [x] 08. OnPossess-Hook integriert
- [x] 09. OnUnPossess-Hook integriert
- [x] 10. Tick im Controller deaktiviert
- [x] 11. Tagesablauf-Komponente ermitteln
- [x] 12. OnRoutineChanged abonnieren
- [x] 13. OnRoutineChanged abmelden
- [x] 14. Aktuelle Zielmarke abfragen
- [x] 15. Zielwechsel-Callback implementiert
- [x] 16. Navigation per Actor-Tag
- [x] 17. Mehrere Tag-Ziele unterstützt
- [x] 18. Nächstes Ziel auswählbar
- [x] 19. Eigenen Pawn als Ziel ausgeschlossen
- [x] 20. Ungültige Zielakteure übersprungen
- [x] 21. Fehlende Welt abgesichert
- [x] 22. Fehlender Pawn abgesichert
- [x] 23. Leere Zielmarke abgefangen
- [x] 24. NavMesh-MoveToActor verwendet
- [x] 25. Navigationserfolg überprüft
- [x] 26. Akzeptanzradius begrenzt
- [x] 27. Aktive Zielmarke gespeichert
- [x] 28. RefreshDestination Blueprint-fähig
- [x] 29. TravelToTag Blueprint-fähig
- [x] 30. AI-Kategorie im Editor angelegt
- [x] 31. NPC um Tagesablauf-Komponente ergänzt
- [x] 32. NPC um Interaktions-Komponente ergänzt
- [x] 33. NPC um Dialog-Komponente ergänzt
- [x] 34. Bestehende Feind-KI nicht überschrieben
- [x] 35. Zivilisten-Controller per Blueprint wählbar
- [x] 36. Interaktion an bestehende Taste gebunden
- [x] 37. Kamera als Trace-Ursprung
- [x] 38. Sichtstrahl bis 400 cm
- [x] 39. Eigene Figur aus Trace ausgeschlossen
- [x] 40. Sichtbarkeits-Kollision verwendet
- [x] 41. Entfernung über InteractionComponent geprüft
- [x] 42. Interaktions-Delegate ausgelöst
- [x] 43. Dialogstart bei Dialog-Komponente
- [x] 44. Demo-Interaktion als Fallback
- [x] 45. Interaktion bei Tod blockiert
- [x] 46. Nichtendliche Interaktionsreichweite abgefangen
- [x] 47. Potenzielle Enemy-Target-Validierung korrigiert
- [x] 48. Editor-Konfiguration dokumentiert
- [x] 49. Statische Projektprüfungen ergänzt
- [x] 50. GitHub-fertiges Vollprojekt als ZIP erstellt

## Unreal-Editor-Arbeiten (noch erforderlich)
1. Im Level `NavMeshBoundsVolume` über begehbare Flächen legen; mit P Navigationsflächen prüfen.
2. Für Arbeitsplätze und Orte leere `TargetPoint`-Actors platzieren und Tags wie `Home`, `Inn`, `Workplace`, `Market`, `GuardPost`, `Barracks` vergeben.
3. Nicht-feindliche NPC-Blueprints auf `AAskirNPC` basieren; `AIControllerClass` auf `AskirCivilianAIController` stellen. Bestehende feindliche NPCs unverändert lassen.
4. NPCs in den `Residents`-Array des `AskirVillageDirector` aufnehmen; Tagesplan `UseVillagerSchedule` oder `UseGuardSchedule` initialisieren.
5. Dialog-Nodes auf NPCs befüllen, Sichtbarkeits-Kollision konfigurieren; Interaktionstaste `E` aus `Config/DefaultInput.ini` prüfen.
6. Alle neuen Klassen in UE 5.6 kompilieren; PIE-Navigation, Dialog, Save/Load und Gegner-KI testen.

**Bekannte Grenzen:** Der AI-Controller ist absichtlich nicht automatisch auf alle NPCs umgestellt, weil `AAskirNPC` sowohl feindliche als auch zivile Charaktere unterstützt. Ohne NavMesh, Ziel-Tags und konfigurierte Blueprints findet keine NPC-Bewegung statt. Kein fertiges `.umap` und keine getestete UE-Integration. Die vorhandenen 3D-Modelle bleiben Prototypen.
