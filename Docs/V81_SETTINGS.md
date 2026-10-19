# V81: Einstellungen und sichere Bildschirmvorschau

## Implementierung

`UAskirGameUserSettings` erweitert die nativen Unreal-Einstellungen um Gesamtlautstärke, Kameraempfindlichkeit und invertierte vertikale Kamerasteuerung. Die eigenen Werte werden in `GameUserSettings.ini` gespeichert; sie gehören nicht in einen Charakter-Spielstand. Grafikqualität, Auflösung, Fenstermodus, VSync und Bildratenbegrenzung verwenden die vorhandenen Unreal-Funktionen.

Eine Bearbeitung besitzt genau einen ursprünglichen Zustand. Nichtgrafische Änderungen können sofort in der Sitzung angezeigt werden. Auflösungs- und Fenstermoduswechsel benötigen eine Bestätigung innerhalb von 15 Sekunden. Die Zeit basiert auf `FPlatformTime`, und ein eigener Core-Ticker überwacht die Frist auch bei pausiertem Spiel oder ohne aktives HUD. Abbrechen und Zeitablauf stellen Lautstärke, Kameraoptionen, jede individuelle Qualitätsstufe, VSync, Bildratenbegrenzung und Videoeinstellungen wieder her. Wiederholtes Öffnen ersetzt den ursprünglichen Zustand nicht. Der HUD-Abbau bricht eine eigene laufende Bearbeitung ab.

Vorschauen rufen `ApplyNonResolutionSettings` und `ApplyResolutionSettings` einzeln auf. `UGameUserSettings::ApplySettings` würde bereits speichern und wird deshalb für Vorschauen nicht verwendet. Die überschreibende `SaveSettings`-Methode blockiert auch externe Speicherversuche während einer offenen Bearbeitung. Vor dem Speichern werden sämtliche Eingaben geprüft; NaN, Unendlich, unbekannte Fenstermodi, ungültige Auflösungen und überhöhte Werte werden verworfen.

Nach dem Speichern werden die tatsächlichen INI-Dateien neu gelesen und die gespeicherten Werte geprüft. Die INI-Dateien und ihr Konfigurationscache werden vorher gesichert. Bei einem fehlgeschlagenen Vergleich wird die Wiederherstellung versucht; die Oberfläche bleibt bei einem Fehler geöffnet. Wiederherstellung und Betriebssystem-Schreibdauerhaftigkeit sind bei einem defekten oder gesperrten Dateisystem nicht garantiert. Ein solcher Fehler darf nicht als erfolgreicher Speichervorgang angezeigt werden.

Die Gesamtlautstärke verwendet einen eigenen transienten SoundMix über der konfigurierten Wurzel-SoundClass einschließlich ihrer Unterklassen. Vorhandene Projektsounds verwenden die Standardklasse. Es wird kein angeblich vorhandenes Projektasset referenziert. Jede neue lokale Spielerwelt wendet den Mix erneut an, weil ein Audio-/Weltwechsel aktive Mischungen zurücksetzen kann. Unabhängige, künftig hinzugefügte SoundClass-Wurzeln benötigen ebenfalls einen Master-Mix oder eine Einordnung in diese Hierarchie.

## Nachweise und verbleibende Prüfung

- **Bestanden:** Portable Produktionsregeln einschließlich Kameraachsen, Wertebereiche, Fristen und Bearbeitungszustandsmaschine: 59.759 Prüfungen mit g++, `-Werror` und UndefinedBehaviorSanitizer.
- **Blockiert:** Die drei nativen Automation-Tests benötigen UE 5.6: Entwurfsvalidierung, abgelehnte Aufrufe ohne Sitzung und tatsächliche Übernahme/Rücknahme der Live-Einstellungen.
- **Blockiert:** Unreal-Kompilierung, Audiogerät, persistente INI-Dateien, tatsächliche Bildschirmmodi und Neustart auf Windows. Die Quellen belegen diese Laufzeitfunktionen noch nicht.

In UE 5.6 zusätzlich prüfen:

1. Menü öffnen, Lautstärke und Kamera ändern, abbrechen; sämtliche ursprünglichen Werte müssen zurückkehren.
2. Auflösung/Fenstermodus ändern, 15 reale Sekunden bei pausiertem Spiel warten; Ausgangsbild und alle übrigen Einstellungen müssen wiederhergestellt werden.
3. Eine Vorschau bestätigen, Anwendung neu starten und die INI-Werte mit den tatsächlichen Einstellungen vergleichen.
4. Nicht beschreibbare INI-Datei simulieren; Speichern muss einen Fehler liefern und den bisherigen Inhalt erhalten, soweit das Dateisystem die Wiederherstellung zulässt.
5. Von Dornfeld ins Umland und zurück reisen; die eingestellte Lautstärke muss weiter gelten.
6. Zwischen Fenster, randlosem Fenster und Vollbild wechseln; eine nicht unterstützte Auflösung muss spätestens durch Abbrechen oder Fristablauf zurückgenommen werden.
