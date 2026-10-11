# Askir RPG V63 – Boss-Kampf-Integrationspflege

Aufbau auf V62, vollständiges Quellprojekt.

## Tatsächliche Änderungen
- Bei Wiederverwendung des Boss-KI-Controllers wird der Damage-Delegate vom vorherigen Boss vor dem erneuten Possess entfernt.
- Eine neue Attack-Telegraph-Vorwarnung wird nicht mehr durch einen zusätzlichen zeitlichen Telegraph-Cooldown unterdrückt. Die KI selbst hat weiterhin ihren AttackInterval.
- Abgebrochene ausstehende Angriffe setzen das AnimNotify-Timing zurück.
- Offline-Validator und Regressionstests ergänzt.
- 50 bossbezogene Regression-Szenarien dokumentiert; keine 50 neuen unabhängigen Gameplay-Features.

## Unreal 5.6
Projekt unter Windows mit UE 5.6 öffnen, C++ mit UBT bauen, Blueprint-Assets per Editor-Skripten erzeugen (DRY_RUN zuerst), Boss-AI-Controller zuweisen, AnimMontages/Notify manuell verbinden, PIE und Packaged Build testen. Keine .uasset-Dateien offline generiert.

## GitHub
Nicht synchronisiert. Upload erfordert funktionsfähige Schreibberechtigung des GitHub-Connectors.
