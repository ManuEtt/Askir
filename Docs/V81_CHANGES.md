# V81 – Hauptmenü und Einstellungen

Ausgangsbasis ist der gesicherte V80-Stand `0ce4f78`. Der unveränderte
V80-Projektstand liegt zusätzlich als ZIP und Git-Bundle außerhalb des Projekts.

## Tatsächlich implementiert, Engine-Ausführung offen

- Native UMG-Titelansicht, Sitzung fortsetzen, Neustartbestätigung, validierter
  Spielstand, Pause → Hauptmenü und Einstellungsansicht.
- GameInstance-Frontend hält beim Neustart einen starken Snapshot und die
  bisherigen Weltbestände vor. Ein Zurücksetzen erfolgt erst nach bestätigter
  Zielwelt und kontrolliertem Spieler. Fehler gehen in die bestehende
  Reise-Wiederherstellung über oder belassen die ursprüngliche Live-Sitzung.
- Eigene `UGameUserSettings`-Klasse, Konfigurationsregistrierung, validierte
  Werte und direkt angeschlossene Kameraachsen. SoundMix steuert die
  Gesamtlautstärke an der konfigurierten SoundClass-Wurzel.
- Anzeigevorschau mit 15-Sekunden-Frist, Beibehalten/Zurücksetzen, unabhängiger
  Echtzeituhr und Rücknahme beim Abbau des Widgets. Speichern liest die
  tatsächliche Konfigurationsdatei zurück; Schreibfehler werden nicht als
  erfolgreicher Abschluss gemeldet.
- Weltbereitschaft und Frontend-Bereitschaft werden getrennt erfasst, damit
  ein pausiertes Menü keine ausstehende Ankunftsinitialisierung blockiert.
- V81-Buildrunner verwendet die V80-Karten-/Assetpipeline und verlangt zusätzlich
  den passenden Titelmenü-Marker nach der Weltbereitschaft. Veraltete Startlogs
  werden abgewiesen.

## Geprüftes und Grenzen

Portable Frontend- und Einstellungsregeln werden unmittelbar vom Produktionscode
verwendet und mit g++/UBSan ausgeführt. Die Python-Tests prüfen unter anderem,
dass Laufzeitnachweise ohne passendes Menü oder mit veralteten Logs abgewiesen
werden. Diese Prüfungen ersetzen keinen UHT-/C++-Build und keine UMG-Bedienung.

Keine neuen Unreal-Karten oder Unreal-Binärassets wurden erzeugt. Die vorhandenen
Originalmodelle und Audiodateien bleiben erhalten. Das Saveformat bleibt 5;
Nutzereinstellungen liegen separat in `GameUserSettings.ini`.
