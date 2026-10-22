# V84 – Ruriks Kräuterauftrag im Dialog

Der bestehende Rurik-Dialog `D_DORNFELD_RURIK` behält seine Identität, sieben
ursprünglichen Dorfkunde-Knoten und die gespeicherte Brunnenkunde. In der ersten
und der erinnerten Begrüßung erscheint eine dem echten SQ_003-Stand entsprechende
Kräuterantwort. Vier weitere Knoten führen zur Annahme, Erinnerung, Rückgabe oder
abgeschlossenen Besprechung. `D_SQ_003` erhält zusätzlich einen vollständigen
Questgraphen mit Antworten für die gespeicherte Hilfe-/Handelsentscheidung.

SQ_003 ist die kanonische Erkundungsquest „Heilkräuter für Mara“: Kräuterwiese im
Dornfelder Umland untersuchen und Rurik berichten. Es werden keine drei Kräuter
oder andere erfundene Inventargegenstände verlangt. Die vorhandene Belohnung
bleibt bei 24 Gold, 150 XP und einmal `item_004`.

`SQ_003_STAGE_AVAILABLE`, `_EXPLORE`, `_RETURN` und `_COMPLETED` sind virtuelle
Bedingungen. Der Cursor entfernt entsprechende Schlüssel aus eingelesenen
Dialogflags und berechnet genau einen anhand des nativen Questledgers. Diese
Schlüssel werden weder vergeben noch gespeichert. Alte oder manipulierte
Dialogflags können deshalb keinen Fortschritt vortäuschen. Dauerhafte
Erzählentscheidungen bleiben unverändert im vorhandenen Flagledger.

Die neuen Aktionen `accept_herbs` und `return_herbs` verwenden ausschließlich die
native Missionstransaktion. Für sie muss der reale freundliche aktive Rurik
`ConversationID=D_DORNFELD_RURIK` und den Editor-Marker
`ASKIR_V84_RURIK_GIVER` besitzen. Eine andere Figur kann durch bloße Zuweisung
des Dialoggraphen keine Questaktion ausführen. Reichweite, Sicht, Weltzustand,
Antwortrevision und offene Transaktionen werden weiterhin geprüft. Die
Rückgabe lädt kanonische Belohnungsdaten bei Bedarf neu und bleibt bei fehlenden
Assets oder vollem Inventar ohne Teilbelohnung gesperrt.

Der bestehende deterministische Generatorname bleibt für die geerbte Pipeline
erhalten. Die Quelldaten `branching_dialogues.json` und `sidequests.json` wurden
nicht geändert. Der kombinierte Katalog enthält 25 Graphen; 22 noch nicht
implementierte Questannahmen sind deaktiviert.

Offline ausgeführt: 36 bestehende Katalogtests, sieben neue Datenprüfungen und
4.890 C++/UBSan-Prüfungen gegen die tatsächlich verwendeten virtuellen
Bedingungen und Aktionsregeln. Drei zusätzliche native Unreal-Tests prüfen
Rurik-Autorisierung, gefälschte gespeicherte Zustände und den vollständigen
Dialogpfad mit voller Tasche, Wiederholungsschutz und erhaltener Brunnenkunde.
Diese nativen Tests sind registriert, aber mangels UE 5.6 nicht ausgeführt.
