# Askir RPG V38 – Themen 851–900

Status bezieht sich auf Quellcode-Änderungen, nicht auf UE-Kompilierung.

851. **Implementiert (ungetestet in UE)** – Persistenten Reisesnapshot vor OpenLevel schreiben
852. **Implementiert (ungetestet in UE)** – Reise bei Save-Fehler abbrechen
853. **Implementiert (ungetestet in UE)** – Portalsperre bei Fehler zurücksetzen
854. **Implementiert (ungetestet in UE)** – GameMode-Speicherfunktion für Portal zugänglich machen
855. **Implementiert (ungetestet in UE)** – Spieler-Inventar über Reise laden
856. **Implementiert (ungetestet in UE)** – Questzustände über Reise laden
857. **Implementiert (ungetestet in UE)** – Questziele über Reise laden
858. **Implementiert (ungetestet in UE)** – Questbelohnungen über Reise laden
859. **Implementiert (ungetestet in UE)** – Gold über Reise laden
860. **Implementiert (ungetestet in UE)** – Charakterfortschritt über Reise laden
861. **Implementiert (ungetestet in UE)** – Ausdauer über Reise laden
862. **Implementiert (ungetestet in UE)** – Gesundheit über Reise laden
863. **Implementiert (ungetestet in UE)** – Dornfeld-Mission über Reise laden
864. **Implementiert (ungetestet in UE)** – Fraktionsruf über Reise laden
865. **Implementiert (ungetestet in UE)** – Dialogflags über Reise laden
866. **Implementiert (ungetestet in UE)** – Weltzustand über Reise laden
867. **Implementiert (ungetestet in UE)** – Tageszeit über Reise laden
868. **Implementiert (ungetestet in UE)** – Quellposition bei Zielkartenladung ignorieren
869. **Implementiert (ungetestet in UE)** – Ankunft am Rückportal beibehalten
870. **Implementiert (ungetestet in UE)** – Reise-Snapshot-Flag nach Laden zurücksetzen
871. **Implementiert (ungetestet in UE)** – Loot-Historie mehrerer Karten beim Speichern erhalten
872. **Implementiert (ungetestet in UE)** – Pro-Karte-Lootstatus wiederherstellen
873. **Implementiert (ungetestet in UE)** – Ungültige Inventarreferenzen beim Laden erkennen
874. **Implementiert (ungetestet in UE)** – Reise-Snapshot-Fehler protokollieren
875. **Offen** – Zielkarte vor Reise auf Existenz prüfen
876. **Offen** – Cook-Map-Whitelist zur Laufzeit validieren
877. **Offen** – Atomaren Snapshot-Schreibpfad implementieren
878. **Offen** – Backup des letzten gültigen Snapshots
879. **Offen** – Transaktionale Rückkehr bei Ladefehler
880. **Offen** – Inventar-Deltas statt Komplettsnapshot
881. **Offen** – Equipment-Objekte persistieren
882. **Offen** – Zauber-Cooldowns persistieren
883. **Offen** – Aktive Buffs persistieren
884. **Offen** – Weltzustände pro Karte persistieren
885. **Offen** – NPC-Tagespläne über Karten persistieren
886. **Offen** – Geöffnete Türen persistieren
887. **Offen** – Besiegte Bosse persistieren
888. **Offen** – Reisebedingungen mit Questflags prüfen
889. **Offen** – Schnellreise-Entdeckungsstatus speichern
890. **Offen** – Reise-UI mit Bestätigung erstellen
891. **Offen** – Ladebildschirm erstellen
892. **Offen** – Asynchronen Levelwechsel untersuchen
893. **Offen** – Autosave-Slotrotation erstellen
894. **Offen** – Versionsmigration für SaveGame
895. **Offen** – Beschädigte Savegames abfangen
896. **Offen** – PIE-Reiseintegrationstest erstellen
897. **Offen** – Packaged-Build-Reisetest erstellen
898. **Offen** – Quest-Reise-End-to-End-Test erstellen
899. **Offen** – Reise zwischen allen 30 Karten testen
900. **Offen** – Profiling der Reise-Ladezeiten durchführen

## Bekannte Grenzen
Der Reise-Snapshot verwendet den festen Slot `AskirTravelSnapshot` (kein transaktionaler/atomarer Save). Die Quellkarte muss den AskirGameMode verwenden. Bei Ladefehlern wird der Wechsel derzeit nicht zurückgerollt. `.umap`-Dateien fehlen weiterhin; Unreal Build und PIE wurden nicht ausgeführt.
