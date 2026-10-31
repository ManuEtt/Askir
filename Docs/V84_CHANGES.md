# V84 – Heilkräuter für Mara

Ausgangsbasis: V83 `ea41fca`. Vor Änderungen liegt die vollständig geprüfte
Projekt-ZIP als `/workspace/backups/AskirRPG_V83_PreV84.zip` gesichert.
Die zwischenzeitlich auf main gemergten Zielerfassungsregressionen aus PR #3
wurden mit `48d135a` übernommen und im Gesamtlauf tatsächlich ausgeführt.

## Implementiert; Engine-Abnahme offen

- SQ_003 aus den bestehenden Daten: Rurik, ein Erkundungsziel und die Rückkehr.
  Hilfe/Handel geben einmalig +2 bei Waldhütern beziehungsweise Freien Händlern.
  Belohnung: 24 Gold, 150 XP und ein `item_004`.
- Nativer Missionscomponent an jeder Spielfigur, persistente eindeutige
  Erkundungsbelege und Reparatur der Journalanzeige aus dem maßgeblichen Ledger.
  Belohnungen nutzen die bestehenden atomaren Inventar-/Gold-/Questtransaktionen.
- Questannahme und Rückgabe an Ruriks bestehende Gespräche angebunden. Die
  Dorfkunde und gespeicherte Brunnenkenntnis bleiben erhalten. Missionsstufen
  werden als flüchtige Bedingungen abgeleitet; gefälschte gespeicherte Stufen
  dürfen keine Questaktion autorisieren.
- HUD und Journal zeigen die neue Mission und die gespeicherte Entscheidung.
- Saveformat 5 bleibt erhalten. Zusätzliche Konsistenzprüfungen kontrollieren
  Annahme, Quelle, Entscheidung, Zielzähler und Belohnung. Speichern/Wiederherstellen
  ist während offener Missionstransaktionen gesperrt.
- Kräuter-Actors binden sich nach EndPlay oder aus einem Zerstörungs-Callback
  nicht erneut an. Bereits gespeicherte Erkundungsbelege bleiben maßgeblich.
- Neues originales Kräutermodell mit echten Blender-/FBX-/GLB-/OBJ-Quellen und
  CC0-Erklärung. Binäre Meshdaten und Importgrenzen werden geprüft.
- Editor-Nachschritt bewahrt die beiden vorhandenen V80-Weltverträge. Er ergänzt
  Ruriks Gebermarkierung sowie ein natives Erkundungsobjekt im Umland und prüft
  nach realem Speichern/Laden Identitäten, Assets, Kollision, Zugang, Navigation
  und einen tatsächlichen ersten Sichtlinientreffer auf dem Interaktionsobjekt.
- Zusätzliche portable Produktionsregeln, Python-Regressionen und native
  Automationstests für Quests, Dialog, Speicherung und den physikalischen Sichttest.

Die ursprünglichen `sidequests.json` und `branching_dialogues.json` bleiben
unverändert. Nur die Laufzeitübersteuerung wird erweitert. 22 weitere deklarierte
Queststarts haben keine vollständige Gameplay-Anbindung und bleiben deaktiviert.

Ohne UE 5.6 und Windows-Host sind UHT/UE-C++-Kompilierung, echte `.umap`-/`.uasset`-
Erstellung, visuelle Bedienung, Durchspielen und Win64-Veröffentlichung blockiert.
Die Offline-Ergebnisse gelten ausschließlich für den dokumentierten Prüfumfang.
