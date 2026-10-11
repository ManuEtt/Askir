# Askir V62 — Angriffszustand und KI-Cooldown

Kumulatives Unreal-Engine-5.6-Quellprojekt auf Basis V61.

## Tatsächliche Codeänderungen
- Boss startet Montage nur, wenn der Charakter nach `Attack()` tatsächlich einen offenen Treffer hat (`HasPendingAttack`).
- Bei gescheitertem Angriff wird der AnimNotify-Modus zurückgesetzt, damit spätere Angriffe nicht im falschen Timing-Modus laufen.
- Basisklassen-KI verbraucht ihr KI-Angriffsintervall nicht mehr bei abgewiesenen Angriffen wegen Stamina/Cooldown.
- Zusätzlicher World-Guard vor dem Sichtbarkeitstrace beim Ende des Windups.
- 50 dokumentierte Regression-Szenarien für fünf Bossprofile; nicht als 50 in Unreal ausgeführte Tests zu verstehen.

## Offene Punkte
Kein Unreal Build Tool, kein PIE-Test, keine echten Montage-Assets. Die UE Editor Skripte bleiben standardmäßig DRY_RUN=True. GitHub wird durch dieses ZIP nicht geändert.
