# Askir RPG V6 – Synchronisierte Kampftreffer

## Zehn Änderungen
1. Schaden aus `PerformAttack` in `ResolvePendingAttackImpact` verlagert.
2. Ausstehende Angriffe werden gespeichert.
3. Doppelter Treffer durch wiederholten Notify wird verhindert.
4. Separater Trefferzeitpunkt für leichte Angriffe.
5. Separater Trefferzeitpunkt für schwere Angriffe.
6. Timer-Fallback ohne Animationsmontage.
7. Native `UAskirAttackImpactNotify` hinzugefügt.
8. Animationsmontagen können den Treffer am Kontaktframe auslösen.
9. Offene Treffer werden beim Tod abgebrochen.
10. Bisherige Sweep-Kollision, Block-/Resistenzberechnung und Treffer-Feedback bleiben bestehen.

## Unreal Engine 5.6
1. C++-Projekt kompilieren und Editor öffnen.
2. Animation Blueprint und Attack-Montages aus V5 einrichten.
3. Einmal `Askir Attack Impact` an der Treffermarke jeder Attack-Montage setzen.
4. Danach am Charakter `bUseAnimationNotifyForImpact=true` einstellen.
5. Solange keine Montages vorhanden sind, `false` lassen: Timer-Fallback ist aktiv.
6. `LightImpactDelay` und `HeavyImpactDelay` bei Bedarf anpassen.
7. In PIE einzelne Treffer, Doppelauslösung, Fehlschläge, Hindernisse und Tod prüfen.

## Noch offen
Nicht mit Unreal 5.6 kompiliert oder in PIE getestet. Es bleibt ein Sphere Sweep an einem Zeitpunkt, kein vollständiger Schwertbogen. Nicht für Multiplayer abgesichert. Animationsdateien, Montages und Skeleton müssen noch im Editor eingerichtet werden.
