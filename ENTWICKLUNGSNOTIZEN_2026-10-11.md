# Entwicklungsstand 11.10.2026

Basis: vom Nutzer bereitgestellte ZIP `AskirRPG_Aktueller_Entwicklungsstand_UE5_6.zip` (55 Einträge). Alle ursprünglichen Dateien übernommen, drei bestehende Dateien erweitert (Charakterheader, Charakter-C++, README), zwei neue C++-Dateien ergänzt und diese Notiz hinzugefügt.

## Grenzen
- Kein UE-5.6-Compiler vorhanden; keine Aussage über erfolgreichen Build.
- Erfahrungspunkte werden noch nicht automatisch von Quests/Gegnern vergeben.
- Fortschritts-Snapshot ist noch nicht an `UAskirSaveGame` angebunden.
- Skills/Perks/Rezepte sind generische Gameplay-APIs, noch ohne UI, Assets und Effekte.
- Charakter-Ruf ist noch nicht mit dem bereits vorhandenen Fraktionssystem synchronisiert.
- Integration bewusst minimal-invasiv; vorhandene Gameplay-Systeme bleiben erhalten.
