# Askir RPG V61 – Boss combat safety and verification

Kumulativer Stand: V60 GitHub Recheck Fixed + V61. 50 nummerierte **Prüfszenarien** 1801–1850 für fünf Bosse (10 je Boss); **nicht** 50 eigenständige Features.

## Funktionale Änderungen
1. Der AnimNotify-Watchdog **bricht** einen nicht aufgelösten Treffer ab, statt nach 2+ Sekunden unerwartet Schaden auszulösen. `OnAttackMissed` signalisiert das Auslaufen.
2. Positive Treffer können optional auch einen bereits gestarteten Bossangriff mit noch ausstehendem Trefferfenster unterbrechen; `bAllowCommittedAttackInterrupt` ist konfigurierbar.
3. Die bisherige Montage-Unterbrechung beendet die ausstehende Schadensauflösung.

## Grenzen
C++ wurde nicht mit Unreal Engine 5.6 kompiliert. Editor-Blueprints und Animations-Montagen fehlen weiterhin als echte `.uasset`-Dateien. Die 50 Szenarien sind eine datenbasierte Prüfliste, keine 50 UE-Integrationstests. GitHub wurde nicht geändert.

## Test
`python -m unittest discover -s Tests -p 'test_*.py'`
