# ASKIR RPG V77 – Verifikation und Review

Stand: 12. Oktober 2026. Dieser Bericht bewertet den V77-Arbeitsstand in `/workspace/Askir`. Die ursprüngliche V76-Inventur bleibt unverändert in `V77_TECHNICAL_AUDIT.md` und `Reports/V77_inventory.json` dokumentiert. Parallel weitergeführte Änderungen müssen vor Lieferung erneut durch den Gesamtprüflauf gehen.

## Tatsächlich ausgeführte Prüfungen

| Prüfung | Status | Ergebnis und Grenze |
|---|---|---|
| Python-Gesamtsuite | **Bestanden** | `python3 -m unittest discover -s Tests -p 'test_*.py'`: **447 Tests**, 0 Fehler, 0 Fehlschläge; Laufzeit im letzten Lauf 0,078 s. Daten-/Python-/Quellvertragsprüfungen, keine UE-Spieltests. |
| Release-Strukturvalidator | **Bestanden** | `python3 Tools/validate_release.py`: Dateistruktur, Rawasset-Bestand, reflektierte Namensdefinitionen, GeneratedHeader-Reihenfolge, einfache UPROPERTY-Deklarationen. |
| Validator-Negativkontrolle | **Bestanden** | Derselbe V77-Validator gegen die isolierte Original-V76-Basis meldet erwartungsgemäß **9 strukturelle Fehler**: sieben Header mit UPROPERTY-Mehrfachdeklarationen sowie zwei doppelte Typdefinitionen. |
| Portable C++-Arithmetik | **Bestanden gemäß ausführendem Teilbericht** | `Reports/V77_system_tests.json`: 511 Assertions gegen den produktiv verwendeten `AskirCheckedMath.h`, C++17, Warnungen als Fehler, UndefinedBehaviorSanitizer. Dieser Review hat den Lauf nicht erneut wiederholt. |
| UHT / vollständiger Unreal-C++-Build | **Blockiert** | Unreal Engine 5.6, Engine-Header/Bibliotheken und UBT fehlen. Eine Textprüfung ist keine Kompilierung. |
| Native UE-Automation | **Blockiert** | Neue Testquellen vorhanden; keine Engine-Testausführung. |
| Editor-Map-Erzeugung/-Laden/MapCheck | **Blockiert** | Kein Unreal Editor verfügbar; Python-/C++-Generierungsquellen zählen nicht als erzeugte `.umap`. |
| Dorfmission mit echten Eingaben/Actors | **Blockiert** | Kein Engine-Lauf; die komplette Sequenz und 90 Gold/180 XP sind nicht als gespielt abgenommen. |
| Win64-Cook/Paketierung/Paketstart | **Blockiert** | UE/UAT und Windows-Build-/Testumgebung fehlen. |
| FPS-/Ladezeit-/Speichermessungen | **Nicht ausgeführt** | Keine behaupteten Messwerte oder erfüllten 60-FPS-Ziele. |

Der zu Beginn vorliegende Zwischenlauf hatte 444 Tests und zwölf Fehlschläge. Diese zwölf Fehlschläge betrafen veraltete Quellassertions: Snapshot-Version 2, mittlerweile ausgelagerte Validation, Leerzeichen in Save-Zuweisungen, alte Variablennamen für Transaktionsgrenzen und die ehemalige Claim-nach-Callback-Reihenfolge. Sie wurden an die tatsächlichen V77-Verträge angepasst. Keine Regression wurde durch bloßes Löschen einer Assertion beseitigt; drei zusätzliche Schutzprüfungen erhöhen die Suite auf 447 Tests.

## Geänderte Regressionen

- `Tests/test_release.py` prüft weiterhin Merchant-/Player-Goldüberlauf, Restock-Grenzen, Kauferschwinglichkeit und Inventarzählsättigung. Die neue zentrale `CanCredit`-Prüfung wird mit ihrer int64-Grenzberechnung verknüpft. Buy-Checks prüfen die Reihenfolge vor dem Ledger-Commit und den Reentrancy-Schutz.
- `Tests/test_v40_equipment_snapshot.py` prüft die fünf Ausrüstungsfelder, das vollständige Capture/Restore, Version 3 mit abwärtskompatibler Versionsgrenze, endliche/nichtnegative Stats und Haltbarkeit 0–100. Zusätzlich müssen Itemauflösung und Snapshot-Validation vor Spielzustandsänderung stattfinden. Die beiden alternierenden Save-Banks werden vor Auswahl/Commit validiert und nach Schreiben rückgelesen.
- `Tests/test_dornfeld_persistence_v76.py` verlangt die persistente Trigger-Claim **vor** Progress-Delegates, damit ein reentrant ausgelöster Overlap nicht doppelt zählt. Beide Claims werden bei fehlgeschlagenem Questfortschritt zurückgenommen. Capture/Restore des Trigger-Ledgers bleibt geprüft.
- `Tests/test_encounter_persistence_v66.py` prüft Capture/Restore weiterhin unabhängig von Leerzeichen. Completion-Dedup, Restart-Gate und die 50 vorhandenen Szenariodatensätze bleiben unverändert geprüft.
- `Tools/validate_release.py` erkennt nun die beiden ursprünglichen Typkollisionen und einfache mehrfache primitive Deklarationen pro UPROPERTY; bei reflektierten Headern muss die generierte Datei zuletzt inkludiert sein. Der Erfolgsoutput nennt ausdrücklich den Offline-Umfang und fehlende Unreal-Kompilierung.

Diese Quellvertragstests prüfen Anschlussstellen und Schutzreihenfolgen. Die tatsächliche Semantik von UE-Objektlebenszeit, Reflection, Delegates, Physik, NavMesh und Dateiserialisierung benötigt native Tests.

## Review von Save/Game, Inventar/Crafting und Production

Die Prüfung des aktuellen Quellstands fand keine erneuten globalen Typnamenskollisionen, keine GeneratedHeader-Includes vor weiteren Includes, keine einfachen UPROPERTY-Mehrfachfelder und keine privaten Blueprint-Properties ohne AllowPrivateAccess. Die öffentlichen Attack-/Notify-Anschlussstellen sind im V77-Header vorgesehen. Diese Aussage ist auf Quellstruktur begrenzt.

Save/Game verwendet eine gemeinsame Capture-/Restore-Route, Version-3-Snapshots, vollständige Validation vor Mutation, Itemauflösung, begrenzte Inventory-Slots und eine sitzungsbezogene regionale Loot-Ledger. Levelwechsel hält das Snapshot-Objekt im GameInstanceSubsystem; Ankunft wird nach dem Pawn-Spawn restauriert. Die alten ConstructorHelpers-Aufrufe wurden durch reguläres Asset-Loading ersetzt. Der neue Character-Lifecycle-Restore ist in `RestoreSnapshot` angebunden. Fehler bei Travel/Restore bleiben explizit sichtbar; zuverlässige Slotdateien und Ankunftspositionen sind noch nicht in Unreal nachgewiesen.

Inventar/Crafting verwenden vorbereitete Austauschlisten und übernehmen sie erst nach Bestands-/Mengen-/Slotprüfungen. Rezeptmengen werden vor Multiplikation geprüft; doppelte Rezept-IDs werden abgewiesen. Merchant-/Inventory-Ledgers sind während Commit und Delegates gegen Reentrancy gesperrt. Das ist konkrete Laufzeitimplementierung im Quelltext; die nativen Inventory-/Crafting-/Trade-Tests müssen sie mit echten Unreal-Objekten noch ausführen. Datenkatalog-/Rezept-/UI-Integration ist gesondert nachzuweisen.

Production-Helpers trennen Editor-APIs über `WITH_EDITOR` und eine konditionale UnrealEd-Abhängigkeit. Sie erzeugen vorgesehene NavVolume-Brush-Geometrie, konfigurieren WorldSettings und prüfen Spawn-Boden mit einem echten Trace. **Diese neuen BrushBuilder-/Navigation-/Python-Reflection-APIs bleiben UE5.6-Kompilier- und Ausführungsrisiken, bis der erste Engine-Lauf sie bestätigt.**

Für vollständige Typen bei regulärem Asset-Loading beziehungsweise World-Aufrufen wurden explizite Includes `Engine/StaticMesh.h` in GameModeDemo und `Engine/World.h` in WorldTravelSubsystem empfohlen. Die übergeordnete Integration übernimmt diese Quellen; dieser Review ändert keine fremd verantworteten C++-Dateien.

## Offene Abnahme

Es gibt keine Freigabe als fertiges oder spielbares Windows-Spiel. Die mindestens 30 echten, getesteten Karten, komplette Kampagne, 20 spielbaren Nebenquests, drei tatsächlich erreichbaren Enden, 24 vollständigen Zauber und der separate Windows-Build bleiben offen. Originalartefakte, echte neue Rohasset-Dateien und Quelländerungen können geliefert werden; nicht erzeugte Unreal-Assets oder Pakete dürfen nicht als vorhanden aufgeführt werden.

## Abschließender Paketstand

Die nachfolgenden Zahlen aus der Reviewphase bleiben als historischer Nachweis erhalten. Die aktuelle Gesamtprüfung nach Spieler-, Audio-, Pipeline- und Persistenzintegration steht in `Reports/V77_verification.json`: 452 Python-Tests, 73 Produktions-Missionsprüfungen und 511 Rechenprüfungen erfolgreich; 19 registrierte Unreal-Automationstests nicht ausgeführt (Engine fehlt). Sieben WAV-Quellen wurden zusätzlich per `--validate-only` geprüft. Unreal-Spielbarkeit und Windows-Build bleiben blockiert.
