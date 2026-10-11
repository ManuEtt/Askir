# Askir RPG V7 – Gegner-KI: 10 Entwicklungsschritte

1. Blueprint-fähiger `AAskirEnemyAIController`.
2. Zustände Patrol, Investigate, Chase, Attack, Retreat, ReturnHome, Dead.
3. Sichtkegel mit Reichweite und LineTrace (Wände blockieren Sicht).
4. `HearNoise`-Funktion mit Geräuschposition und Lautstärke.
5. Zielgedächtnis mit letzter bekannter Position und Ablaufzeit.
6. Patrouillenrouten über `SetPatrolPoints`.
7. NavMesh-basierte Verfolgung.
8. Nahkampfangriff über `AAskirCharacter::Attack` mit Cooldown.
9. Rückzug bei geringer Gesundheit.
10. Verfolgungsgrenze und Rückkehr zum Ausgangspunkt.

## Unreal Engine 5.6: Einrichtung
- Visual Studio 2022 / Unreal Build Tool: C++-Projekt kompilieren.
- `BP_AskirBandit` als Blueprint-Kind von `AAskirCharacter` erstellen.
- `AI Controller Class = AskirEnemyAIController`; `Auto Possess AI = Placed in World or Spawned`.
- `NavMeshBoundsVolume` in Testlevel platzieren; mit **P** begehbare Flächen prüfen.
- NPC platzieren und bei Bedarf `SetPatrolPoints` mit Weltkoordinaten aufrufen.
- `HearNoise` bei Fußschritten/Waffengeräuschen von Blueprint auslösen.
- Mit PIE Sicht, Verfolgung, Suche, Rückzug, Nahkampf, Tod und Heimkehr prüfen.

## Grenzen
- Noch **nicht** in Unreal Engine kompiliert oder in PIE getestet.
- KI ist auf Singleplayer (`PlayerCharacter(0)`) ausgelegt.
- Keine Behavior Trees, Blackboard-Assets, EQS oder automatische Geräuschverteilung.
- Keine `.uasset`-Blueprints oder Level-Navigation in der ZIP erstellt.
- Das bestehende Kampfsystem und die Rigging-/Animationspipeline benötigen weiterhin Unreal-Tests.
- Der Controller verwendet `AAskirCharacter`; eine dedizierte NPC-Klasse ist ein sinnvoller nächster Schritt.
