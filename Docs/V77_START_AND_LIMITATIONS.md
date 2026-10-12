# V77 – Start, Inhalt und verbleibende Blockaden

Alle V76-Quellen und Rohmodelle wurden bewahrt, soweit sie nicht gezielt repariert oder zur Vermeidung von UHT-Kollisionen umbenannt wurden. Die Originaldateiliste mit SHA-256 steht in `Reports/V77_inventory.json`. V77 ergänzt Missions-/Kampf-/Persistenz-/Wirtschaftsreparaturen, Enhanced Input, native UMG, sieben Audioquellen, Unreal-Generierung, Buildrunner und Tests.

## Unreal-Ausführung auf Windows

Benötigt werden Windows 64 Bit, Unreal Engine 5.6, deren erforderliche Visual-Studio-C++-Buildwerkzeuge und Windows SDK sowie Python 3.10+. Die Editor-Plugins für Python und Editor Scripting sind im Projekt aktiviert. Engine-, Compiler- und Lizenzdaten sind nicht enthalten.

1. ZIP in einen beschreibbaren Ordner entpacken. Der Ordner mit `AskirRPG.uproject` ist die Projektwurzel.
2. PowerShell dort öffnen und den vollständigen Beispielbefehl aus der README ausführen. Der Ausgabeordner muss neu und außerhalb des Projekts sein.
3. Compile-/Generierungs-/Testberichte prüfen. Die Pipeline stoppt bei Fehlern; Offline-Tests belegen keine Unreal-Kompatibilität.
4. Erst nach echtem Speichern und erneutem Laden der Dorfkarte wird der Standardstartlevel auf `Dornfeld_V77` gesetzt. Hier bleibt die Engine-Entry-Karte erhalten, weil noch keine Map erzeugt wurde.
5. Nach erfolgreichem Build das Spiel zusätzlich mit Grafik und Ton starten und die Mission manuell durchspielen: Mara, drei Scherben, zwei Wächter, Kellertruhe und Abgabe. Speichern/Laden nach Scherben, einem Wächter und abgeschlossener Mission wiederholen. Der automatische Starttest ohne Grafik ersetzt dies nicht.

`-CheckOnly` prüft Werkzeuge. `-UseExistingMap` erhält und prüft eine bereits generierte Dorfkarte. Die Pipeline sichert Content, Source und Config. Native Automation Tests sind unter `Askir.*` registriert. Git LFS ist für zukünftige `.uasset`, `.umap` und `.fbx` vorbereitet.

## Implementierte Steuerung

WASD Bewegung; Maus Kamera; Umschalt Sprint mit Ausdauer; Leertaste Sprung; linke/rechte Maustaste leichter/schwerer Angriff; Alt Ausweichen; Q Block; E Interaktion; Tab Inventar; J Journal; Esc Pause; F5/F9 Speichern/Laden. Beide Speicherzugriffe und das Pausemenü verwenden `AskirQuick` mit BankA/BankB.

Mara zeigt bestätigbare Annahme und Abgabe. Drei unterschiedliche Scherben aktivieren die Wächter; zwei angerechnete Spieler-Kills öffnen die Truhe; Mara vergibt einmal 90 Gold und 180 XP. Der Dorfkartengenerator setzt Wächter-XP auf null. Sichtbare Figuren sind primitive Stellvertreter und keine fertig animierten Produktionscharaktere.

## Nachgewiesene Grenzen

- Linux-Umgebung ohne UE 5.6: UHT, Unreal-Kompilierung, Editor-Scripts, native Automation, Cook und Windows-Paket wurden nicht ausgeführt.
- Null `.umap` und null `.uasset`. Die 30 bestehenden Regionen bleiben Daten/Generierungspläne; keine ist als spielbarer Level abgenommen.
- Kampagne, Nebenquests, Enden, Magie, Produktionsmodelle/Rigs/Animationen und vollständige Benutzeroberfläche sind nicht fertig. JSON-Daten zählen nicht als spielbare Inhalte.
- Performance/FPS wurden nicht gemessen; kein lauffähiger Unreal-Build steht hier bereit.
- Sieben WAV-Quellen wurden erzeugt und technisch geprüft. Unreal-Import, Wiedergabe und Mischung bleiben ungeprüft.
- GitHub verweigerte die Branch-Erstellung mit HTTP 403 (`Resource not accessible by integration`). Kein Remotecommit oder erfolgreicher Upload wurde bestätigt. Lokaler Commit und ZIP sichern die Änderungen.

Audit, aktuelle Test-JSON und Änderungskapitel dokumentieren die offenen Abnahmen. Dieses Archiv dient der Weiterentwicklung und Engineprüfung.
