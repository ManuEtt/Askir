# V83: Validierte Gesprächsdaten

Der Gesprächskatalog übernimmt alle 24 vorhandenen Einträge aus `Production/Dialogue/branching_dialogues.json`. Diese Ausgangsdatei und die Questdaten werden durch den Generator nicht verändert. `runtime_overrides_v83.json` ersetzt ausdrücklich Torbens Gespräch und ergänzt einen eigenständigen Dialog mit Rurik. Das ergibt 25 Datengraphen, keine 25 spielbaren Quests.

Torben verwendet weiterhin den vorhandenen Auftrag SQ_001: zwei Banditen, Lieferung sichern, Rückkehr; Belohnung 16 Gold, 100 Erfahrung und `item_002`. Die Dialogaktionen rufen die abgesicherten Missionsmethoden auf. Der Katalog vergibt selbst keine Belohnungen. Anfangsknoten hängen vom tatsächlichen Missionszustand ab. Nach dem Abschluss reagiert Torben unterschiedlich auf die gespeicherte Entscheidung für die Waldhüter oder die Freien Händler. Die freiwillige Frage nach den Handelswegen speichert `TORBEN_ROAD_HISTORY` und ermöglicht bei einem späteren Gespräch eine erinnerte Antwort.

Ruriks sieben Gesprächsknoten behandeln Ardem, Falkenwacht, den Grauwald, den Dorfbrunnen und die fünf Siegel der zerbrochenen Krone. Grundlage sind `Production/Lore/world.json` und Kapitel 1 aus `Production/Story/main_story.json`. Rurik unterscheidet ausdrücklich zwischen Überlieferung und überprüften Ereignissen. Die Frage nach dem Brunnen speichert `RURIK_WELL_LORE`; beim nächsten Gespräch begrüßt er den Spieler entsprechend. Der Dialog startet keine weitere Quest und vergibt keine Gegenstände, Erfahrung oder Gold.

Die übrigen 23 ursprünglichen Questannahmen bleiben im Katalog sichtbar als ausdrücklich deaktivierte Auswahlmöglichkeiten. Ihre vorhandenen `quest_start`-Deklarationen werden nicht als vollständige Nebenquests ausgegeben. Bevor eine solche Annahme freigeschaltet wird, benötigt sie einen tatsächlich implementierten Spielablauf.

## Generierung und Prüfung

```bash
python Tools/generate_conversation_catalog_v83.py
python Tools/generate_conversation_catalog_v83.py --check
python -m unittest Tests.test_conversation_catalog_v83
```

`--project-root <Verzeichnis>` ermöglicht isolierte Testprojekte. Die Ausgabe liegt in `Source/Askir/Dialogue/AskirConversationCatalog.generated.inl`; `AskirConversationCatalog::Build` befüllt die nativen Gesprächsstrukturen ohne JSON-Dateizugriff während des Spiels. Der Generator schreibt deterministische ASCII-Dateien mit korrekt maskierten Unicode-Zeichen und einem Hash der drei Eingabedateien. `--check` lehnt fehlende oder veraltete Ausgaben ab.

Die Prüfung verwirft ungültiges UTF-8, doppelte JSON-Schlüssel, nicht endliche JSON-Zahlen, unbekannte Felder und Aktionen, doppelte IDs einschließlich Groß-/Kleinschreibung, ungültige Kanten, verwaiste Knoten und unbekannte Questreferenzen. Grenzen: 64 Graphen, je 64 Knoten, 32 Auswahlmöglichkeiten pro Knoten, 32 Einstiegskonditionen und je 16 erforderliche beziehungsweise verbotene Flags. Texte sind ebenfalls begrenzt. `GrantedFlag` darf keine mit `SQ_` beginnenden Missionsbelege erzeugen; diese bleiben Aufgabe des Missionssystems.

Die Tests prüfen beschädigte Eingaben, echte Unicode- und Escape-Zeichen, unveränderte Quelldateien, deaktivierte nicht implementierte Annahmen sowie die Kompilierbarkeit des erzeugten C++ gegen einen portablen Strukturvertrag mit UBSan. Dieser portable Kompiliertest ersetzt keine Unreal-Kompilierung. Unreal-Dialogdarstellung, Actor-Anbindung und Speichern/Laden im Spiel müssen mit UE 5.6 gesondert geprüft werden.
