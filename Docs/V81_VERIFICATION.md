# V81 – Prüfstatus und Abnahme

Dieser Entwicklungsstand wurde auf Linux ohne Unreal Engine 5.6 geprüft.
`Reports/V81_verification.json` ist die maßgebliche maschinenlesbare Ausgabe
für die Offline-Prüfungen; sie enthält jede ausgeführte Prüfung und die
Hashes der dabei unveränderten Quellen. Die Engine-Umgebungsprüfung liegt
unter `Reports/V81/pipeline_environment.json`.

| Prüfung | Status |
| --- | --- |
| Python-Regressionen | Bestanden: 568 Tests |
| Portable C++-Produktionsregeln mit g++ und UBSan | Bestanden: 216.403 Assertions in zehn Programmen |
| FBX-/GLB-/Rig-/Animation-/Prop-Binärquellen | Bestanden |
| Unreal Header Tool und natives UE-C++ | Blockiert: Engine fehlt |
| Native `Askir.*`-Automation | 51 Tests registriert, nicht ausgeführt; Engine fehlt |
| UMG-Darstellung, Maus und Tastatur | Nicht ausgeführt; Engine-Laufzeit fehlt |
| `.umap`-/`.uasset`-Generierung und Import | Blockiert: Engine fehlt |
| Win64-Paket und eigenständiger Spielstart | Blockiert: Engine und Windows-Host fehlen |
| Dorfmission, Nebenquest und Weltwechsel im Spiel | Nicht ausgeführt |
| Performance/60 FPS | Blockiert; keine FPS-Messwerte |

Die zusätzlichen portablen Prüfungen testen die tatsächlich eingebundenen
Einstellungs- und Frontendregeln: numerische Grenzwerte/NaN/Unendlich,
Kameraachsen, Bearbeitungssitzung, Ablauf der Anzeigevorschau, Sessionwechsel
und Bestätigungsvoraussetzungen. Python prüft zusätzlich die Grenzen der
Startnachweise und lehnt alte oder unvollständige Logs ab.

## Noch erforderliche Engine-Abnahme

Auf Windows UE 5.6 und die C++-Werkzeuge installieren, anschließend den im README
beschriebenen V81-Runner in einem neuen externen Ausgabeordner starten. Erst
seine tatsächlich erzeugten Karten, Buildlogs und Automationsergebnisse
können die entsprechenden technischen Kriterien belegen.

Die folgenden manuellen Prüfungen sind **nicht ausgeführt**:

1. Spiel außerhalb des Editors öffnen. Das Hauptmenü muss nach dem kontrollierten
   Spieler erscheinen, ohne dass die Ankunftsinitialisierung blockiert wird.
2. Ohne gespeicherte Partie neues Spiel beginnen, bewegen, Kamera/Interaktion
   testen. Mara-Mission mit drei unterschiedlichen Scherben, zwei Wächtern,
   Kellertruhe und Rückgabe vollständig spielen; genau 90 Gold/180 XP prüfen.
3. Speichern, Prozess beenden und den Spielstand über das Hauptmenü laden.
   Position, Mission, Loot und Werte müssen erhalten bleiben.
4. Torbens Auftrag mit beiden Dialogentscheidungen getrennt spielen, ins Umland
   und zurück reisen; Lieferung und einmalige Belohnung samt Fraktion prüfen.
5. Aus einer laufenden Sitzung zum Hauptmenü wechseln und fortsetzen. Neustart
   zuerst abbrechen, danach bestätigen: neue Live-Welt ohne alte Quest-/Loot-
   bestände; vorhandene Speicherdateien weiterhin ladbar.
6. Einen fehlgeschlagenen Neustart/Weltwechsel provozieren. Die ursprüngliche
   Sitzung oder der gesicherte Snapshot muss über die Wiederherstellung
   erreichbar bleiben. Kein dauerhaft gesperrtes Menü akzeptieren.
7. Lautstärke, Kameraempfindlichkeit/Inversion, Grafikqualität, FPS und V-Sync
   ändern. Abbrechen muss Originalwerte herstellen. Übernehmen und Prozess-
   neustart müssen die bestätigten Werte erhalten.
8. Auflösung/Fenstermodus ändern und 15 Sekunden nicht bestätigen, auch bei
   pausierter Welt: ursprüngliche Anzeige muss zurückkehren. Danach Änderung
   ausdrücklich bestätigen und nach Prozessneustart prüfen.
9. Einstellungen bei fehlenden Dateischreibrechten übernehmen: sichtbare
   Fehlermeldung; keine Erfolgsmeldung und weiter mögliche Rücknahme.
10. Inventar, Magie, Händler und Crafting vor/nach Laden benutzen, Tod/Laden
    prüfen. Bildrate und Speicher an dokumentierter Hardware messen.

Die aktuelle Menü-Implementierung ist textbasiertes natives UMG. Ein visuelles
Art- und Bedienungsreview bleibt Teil der Engine-Abnahme. Eine ZIP mit Quellen
ist kein paketiertes Windows-Spiel und keine Abnahme der Gesamt-Kampagne.
