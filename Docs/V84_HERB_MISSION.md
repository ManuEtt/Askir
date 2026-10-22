# V84: Heilkräuter für Mara (SQ_003)

Die native Mission verwendet Rurik, das kanonische Erkundungsziel `SQ_003_A`
und die Rückkehr `SQ_003_B`. Nach Hilfe-/Handelswahl wird das ausgewiesene
Kräutervorkommen im Dornfelder Umland ausdrücklich untersucht. Nähe allein
genügt nicht. Danach vergibt Ruriks Bericht einmalig 24 Gold, 150 XP und
ein echtes `item_004`; fehlende importierte Belohnungsassets blockieren den
Abschluss. Das Erkundungsziel erzeugt keine vorgezogene Inventarbeute.

Die bestehenden Quest- und Dialogfelder des Saveformats 5 speichern Annahme,
Wahl, Erkundung, Herkunft und Auszahlung. Der Laufzeit-Questdaten-Cache wird
nach Laden/Reisen bei Bedarf rekonstruiert. Ein fehlgeschlagener Abschluss
stellt den vorherigen Zielstand wieder her; ein volles Inventar kann nach
Freimachen erneut versucht werden. Wallet- und XP-Überläufe werden vor
Auszahlung durch die vorhandene atomare Questtransaktion abgewiesen.

`AskirHerbPatchActor` prüft lebenden, kontrollierten Spieler, aktuelle Welt,
Eingabekontext, Entfernung, Sichtlinie und die kanonische Herkunfts-ID.
Ein eigener Query-Box-Collider blockiert ausschließlich Sichtabfragen;
Blattgeometrie blockiert weder Spieler noch Navigation. Geprüfte Erkundung
blendet den Marker aus, die Wiederherstellung folgt dem Questledger.

`AskirHerbRules.h` wird direkt vom nativen Backend und dem portablen
C++-Grenztest verwendet. Native Tests decken tatsächliche Auszahlung,
volle Tasche, Gold-/XP-Überlauf, räumliche Interaktion und Reentranz in
realen Fraktions-/Questdelegates ab. Native Unreal-Ausführung, Editor-
Import und spielbare Level bleiben bis zu einem protokollierten UE-5.6-
Lauf ausdrücklich nicht geprüft.
