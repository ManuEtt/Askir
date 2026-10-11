# Unreal Engine 5.6 – V26 Testplan (noch auszuführen)
1. Projektdateien generieren und Editor-Build kompilieren.
2. Charakter-Blueprint öffnen; GuardComponent auf Vorhandensein prüfen.
3. Testeingabe mit StartGuard/StopGuard verknüpfen.
4. Ohne Block 40 Schadenspunkte verursachen, Health-Differenz prüfen.
5. Mit Block denselben Schaden verursachen, reduzierte Health-Differenz prüfen.
6. Block mehrfach treffen bis GuardBreak, OnGuardBroken prüfen.
7. Sofort nach GuardBreak StartGuard versuchen: muss bis Cooldown fehlschlagen.
8. Stamina-Regeneration im Leerlauf und keine Regeneration während Block prüfen.
9. Negative und Null-Schadenswerte, fehlende Guard-Komponente testen.
10. Spieler, NPCs, Respawn, Save/Load und Animationen manuell regressionsprüfen.
