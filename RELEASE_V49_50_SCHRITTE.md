# Askir V49 – Encounter-Actor-Integration (Themen 1301–1350)

## Implementiert
- Unreal Editor Python: `Tools/integrate_encounter_actors_v49.py`.
- Datengetriebene Anlage von **90 `AAskirEncounterZone`** und **90 `AAskirEncounterSpawner`** Actors in den 30 Karten, sofern diese zuvor erzeugt wurden.
- Spawner/Zone-Verknüpfung, Encounter-ID, Gegnerzahl und optionale Enemy-Blueprint-Klasse werden gesetzt.
- Duplikatschutz anhand stabiler Actor-Labels und Klassenkontrolle.
- Standardmäßig **DRY_RUN=True**; keine Karten werden ohne ausdrückliche Skriptänderung gespeichert.
- Fehlende Maps/Blueprints werden erfasst bzw. bei aktiviertem Modus als Fehler gemeldet.
- Offline-Datenvalidierung und 50 Tests (30 regionale Konfigurationstests + 20 Integrationsverträge).

## Themen 1301–1350
- 1301: Encounter-Integration für `L01_Dornfeld` geprüft (Daten/Tests; UE-Ausführung offen).
- 1302: Encounter-Integration für `L02_Dornfelder_Umland` geprüft (Daten/Tests; UE-Ausführung offen).
- 1303: Encounter-Integration für `L03_Grauwald` geprüft (Daten/Tests; UE-Ausführung offen).
- 1304: Encounter-Integration für `L04_Alte_Brunnenkrypta` geprüft (Daten/Tests; UE-Ausführung offen).
- 1305: Encounter-Integration für `L05_Verlassene_Silbermine` geprüft (Daten/Tests; UE-Ausführung offen).
- 1306: Encounter-Integration für `L06_Falkenwacht` geprüft (Daten/Tests; UE-Ausführung offen).
- 1307: Encounter-Integration für `L07_Nordpass` geprüft (Daten/Tests; UE-Ausführung offen).
- 1308: Encounter-Integration für `L08_Ruinen_von_Eldrath` geprüft (Daten/Tests; UE-Ausführung offen).
- 1309: Encounter-Integration für `L09_Nebelmoor` geprüft (Daten/Tests; UE-Ausführung offen).
- 1310: Encounter-Integration für `L10_Krypta_der_Sieben` geprüft (Daten/Tests; UE-Ausführung offen).
- 1311: Encounter-Integration für `L11_Verlorenes_Kloster` geprüft (Daten/Tests; UE-Ausführung offen).
- 1312: Encounter-Integration für `L12_Kuestenhafen_Ardem` geprüft (Daten/Tests; UE-Ausführung offen).
- 1313: Encounter-Integration für `L13_Hafen_von_Thalor` geprüft (Daten/Tests; UE-Ausführung offen).
- 1314: Encounter-Integration für `L14_Sonnenebene` geprüft (Daten/Tests; UE-Ausführung offen).
- 1315: Encounter-Integration für `L15_Kristallhoehlen` geprüft (Daten/Tests; UE-Ausführung offen).
- 1316: Encounter-Integration für `L16_Vergessene_Bibliothek` geprüft (Daten/Tests; UE-Ausführung offen).
- 1317: Encounter-Integration für `L17_Wuestenruinen_SarKel` geprüft (Daten/Tests; UE-Ausführung offen).
- 1318: Encounter-Integration für `L18_Tempel_des_Zweiten_Siegels` geprüft (Daten/Tests; UE-Ausführung offen).
- 1319: Encounter-Integration für `L19_Grenzfestung_Velis` geprüft (Daten/Tests; UE-Ausführung offen).
- 1320: Encounter-Integration für `L20_Verbrannte_Lande` geprüft (Daten/Tests; UE-Ausführung offen).
- 1321: Encounter-Integration für `L21_Schwarzwald_Velis` geprüft (Daten/Tests; UE-Ausführung offen).
- 1322: Encounter-Integration für `L22_Unterirdische_Zwergenstadt` geprüft (Daten/Tests; UE-Ausführung offen).
- 1323: Encounter-Integration für `L23_Blutsteinminen` geprüft (Daten/Tests; UE-Ausführung offen).
- 1324: Encounter-Integration für `L24_Festung_der_Schattenlegion` geprüft (Daten/Tests; UE-Ausführung offen).
- 1325: Encounter-Integration für `L25_Aschenfeste` geprüft (Daten/Tests; UE-Ausführung offen).
- 1326: Encounter-Integration für `L26_Tal_der_gefallenen_Koenige` geprüft (Daten/Tests; UE-Ausführung offen).
- 1327: Encounter-Integration für `L27_Tempel_des_Vierten_Siegels` geprüft (Daten/Tests; UE-Ausführung offen).
- 1328: Encounter-Integration für `L28_Verlorene_Koenigsstadt` geprüft (Daten/Tests; UE-Ausführung offen).
- 1329: Encounter-Integration für `L29_Heiligtum_der_Krone` geprüft (Daten/Tests; UE-Ausführung offen).
- 1330: Encounter-Integration für `L30_Thronsaal_von_Askir` geprüft (Daten/Tests; UE-Ausführung offen).
- 1331: DRY_RUN – Skriptvertrag und Offline-Test.
- 1332: Map-Existenz – Skriptvertrag und Offline-Test.
- 1333: Map-Laden – Skriptvertrag und Offline-Test.
- 1334: Map-Speichern – Skriptvertrag und Offline-Test.
- 1335: Zone-Klasse – Skriptvertrag und Offline-Test.
- 1336: Spawner-Klasse – Skriptvertrag und Offline-Test.
- 1337: Zone-Label – Skriptvertrag und Offline-Test.
- 1338: Spawner-Label – Skriptvertrag und Offline-Test.
- 1339: Duplikatschutz – Skriptvertrag und Offline-Test.
- 1340: Klassenkollision – Skriptvertrag und Offline-Test.
- 1341: Encounter-ID – Skriptvertrag und Offline-Test.
- 1342: Zone-Zuordnung – Skriptvertrag und Offline-Test.
- 1343: Gegnerzahl – Skriptvertrag und Offline-Test.
- 1344: Wellenzahl – Skriptvertrag und Offline-Test.
- 1345: Spawnradius – Skriptvertrag und Offline-Test.
- 1346: Blueprint-Pfad – Skriptvertrag und Offline-Test.
- 1347: Blueprint-Fehler – Skriptvertrag und Offline-Test.
- 1348: Map-Bericht – Skriptvertrag und Offline-Test.
- 1349: Editor-Actor-Erstellung – Skriptvertrag und Offline-Test.
- 1350: Expliziter Aufruf – Skriptvertrag und Offline-Test.

## Verwendung
1. In UE 5.6 C++ Projekt kompilieren und V42 Karten erzeugen.
2. Optional für jeden Encounter `blueprint_class_path` als Unreal-Klassenpfad (`..._C`) setzen; ohne Gegnerklasse kann der Spawner **nicht** starten.
3. Editor-Python-Skript zunächst mit `DRY_RUN=True` ausführen und Bericht prüfen.
4. Projekt sichern; danach `DRY_RUN=False` setzen und im Unreal Editor erneut ausführen.
5. Im Editor Actor-Verknüpfung und Trigger-Komponenten prüfen; Play-in-Editor, NavMesh, Questlogik, SaveGame und Kampf testen.

## Grenzen
Die Integration wurde hier **nicht in Unreal ausgeführt oder kompiliert**. Die 30 `.umap` Karten liegen nicht im ZIP. Die Enemy Blueprint Klassen fehlen weiterhin. Tests prüfen Python-Verträge und Datensätze, nicht die Unreal-Laufzeit. Es ist kein spielbarer Release-Build.
