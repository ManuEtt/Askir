# V83 – Verzweigte Gespräche und erinnerte Dorfkunde

Ausgangsbasis ist V82 `5dfead3`. Vor Änderungen wurde dessen vollständige,
verifizierte Projekt-ZIP als `AskirRPG_V82_PreV83.zip` außerhalb des Projekts
gesichert. V82 liegt zusätzlich auf GitHub und hat die Offline-CI bestanden.

## Implementiert, Unreal-Ausführung offen

- Nativer Gesprächscursor an der Spielfigur mit validierten Graphen, Bedingungen,
  stabilen Antwort-IDs, monotoner Revision und wiederholter Kontextprüfung.
- Torbens feste HUD-Texte und direkte UI-Questaktionen durch Katalogausführung
  ersetzt. Annahme und Rückgabe verwenden weiterhin die bestehenden
  `AcceptDelivery`-/`ReturnToTorben`-Transaktionen mit ihren Originalbelohnungen.
- Nach Laden oder Rückreise löst die Rückgabeprüfung Torbens kanonische
  Belohnungsdaten erneut auf. Ein leerer Laufzeitcache blockiert die Übergabe
  damit nicht dauerhaft; fehlende echte Gegenstandsassets bleiben gesperrt.
- Zustandsabhängige Einstiege für Banditen, Fracht, Rückgabe und Abschluss.
  Gespeicherte Hilfe-/Handelsentscheidung bestimmt die spätere Antwort.
- Ruriks Gespräch mit sieben Knoten: Dorfkunde, Handelswege, Grauwald und die
  kanonische Verbindung von Krone, fünf Siegeln und Erinnerungen. Die bestehende
  Spieler-Flagliste speichert gelernte Brunnenkunde und verändert die Begrüßung.
- 24 ursprüngliche Dialogvorlagen erhalten, Torben gezielt erweitert und Rurik
  ergänzt. 23 deklarierte Queststarts sind mangels Gameplay-Implementierung
  deaktiviert; sie zählen nicht als spielbare Nebenquests.
- Allgemeiner NPC-Interaktionspfad öffnet nun die Gesprächsoberfläche. Bereits
  konfigurierte Legacy-Knoten werden in den neuen Cursor übernommen; dauerhafte
  Flags stammen aus dem Spielerledger. E-Hinweise berücksichtigen diese NPCs.
- Neue UI-Antwortbuttons prüfen sowohl Menükontext als auch Gesprächsrevision.
  Entfernen/Schließen, Tod, Laden, Weltwechsel und ungültige Gesprächspartner
  beenden den Cursor. Während aktiver Gespräche ist Gameplay-Eingabe gesperrt.
- Reentranzschutz vor Quest-/Fraktionscallbacks, gesperrter Save/Restore während
  einer Aktion und aufgeschobener Gesprächsabbruch. Actor-Zerstörung während
  eines Callbacks lässt keine alten Referenzen oder blockierten Cursor zurück.
- Deterministischer Kataloggenerator mit Schema-/Kantenprüfung, C++-String-
  Escaping und tatsächlichen portablen Syntax-/Unicode-Ausführungstests.
- Editor-Nachschritt ergänzt Rurik in der bestehenden Dornfeld-Karte. Nach
  Speichern/Laden werden Herkunfts-Actors, Mesh/Clips, Bodenkollision und
  vollständiger Navigationspfad geprüft; beide V80-Welten werden nachvalidiert.
- Die Katalogprüfung läuft direkt im eingebetteten Editor-Python. Sie startet
  keinen zweiten Editor; fehlende oder veraltete Ausgaben brechen weiterhin ab.

Spielstände behalten Format 5. Dialogcursor und Actor-Referenzen sind transient;
gespeichert werden weiterhin die vorhandenen Quest-, Fraktions- und Dialogflags.
Es wurden keine neuen externen Assets oder kostenpflichtigen Inhalte bezogen.
Die vorhandenen Guard-Quellen werden für Rurik wiederverwendet.

Die echte Editor-Kartenerweiterung ist mangels Engine nicht ausgeführt. Die
Quellen und Offline-Prüfungen belegen weder UHT-/UE-C++-Kompilierung noch eine
bedienbare Szene oder erfolgreiches Durchspielen.
