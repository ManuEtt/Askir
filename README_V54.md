# Askir V54 – Themen 1451–1500

## Umgesetzt: 90 Begegnungen an Gegner-Blueprints binden

30 regionale Prüfungen, 14 Archetyp-Prüfungen und 7 Vertragsprüfungen (insgesamt 51 Tests).

Die 14 bestehenden Gegner-Archetypen werden nachvollziehbar auf die sechs V53-Gegner-Blueprints abgebildet.

Editor-Skript `Tools/bind_encounter_blueprints_v54.py` setzt EnemyClass an V49-Spawnern, standardmäßig nur Vorschau.

Quest-Abschluss-IDs bleiben bewusst leer: ohne validierte Quest-IDs würden falsche Ziele fortgeschrieben.

## Ausführung

1. UE 5.6 C++ erfolgreich kompilieren und Python Editor Script Plugin aktivieren.
2. Mit V42 die Maps, V51/V53 die Blueprint-Klassen und V49 die Spawner erzeugen.
3. `Tools/bind_encounter_blueprints_v54.py` mit `DRY_RUN=True` prüfen.
4. Nach Sicherung `DRY_RUN=False` setzen und ausführen.
5. Im Editor NavMesh, KI-Besitz, Kampf, Quest-Events und Speichern prüfen.

**Nicht geprüft:** Unreal-Editor-Ausführung, `.uasset`-Erstellung, C++-Kompilierung und tatsächliche Spielbarkeit.
