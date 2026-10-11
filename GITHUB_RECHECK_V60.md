# GitHub-Nachprüfung V60 (11.10.2026)

- Aktueller geprüfter GitHub-Commit: `bdde215f7487d9ab2f44c7ef0b3b5853aa31dba9`.
- Die frühere Audit-Datei und drei korrigierte Editor-Skripte sind auf `main` vorhanden und byteidentisch mit der lokalen Audit-Version.
- Weitere lokale Korrektur: `AAskirCharacter::PerformAttack` erhält bei AnimNotify-basierten Treffern einen Sicherheits-Timer. Ohne Notify blieb zuvor `bPendingAttack` dauerhaft aktiv; nun wird der ausstehende Treffer spätestens nach dem Watchdog aufgelöst. Die bestehende `bPendingAttack`-Prüfung verhindert Doppelauflösung.
- Der Watchdog ist ein Gameplay-Fallback und ersetzt keinen korrekten Montage-Notify.
- Offline-Testlauf und Unreal-Kompilierung sind getrennt zu bewerten. GitHub wurde bei diesem Prüfschritt nicht verändert.
