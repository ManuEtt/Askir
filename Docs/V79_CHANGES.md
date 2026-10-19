# V79 · Händler und Werkbank

## Tatsächlich erstellte Ergebnisse

- Native Händler- und Werkbankinteraktionen mit Reichweiten-, Lebenszustands- und Speicheroperationsprüfung.
- UMG-Kaufen/Verkaufen für jeweils ein Exemplar, Preise, Händlergold, Vorrat und Spielerbestand; nicht erfüllbare Transaktionen sind deaktiviert und werden vor dem Commit erneut geprüft.
- Craftingmenü mit benötigten/vorhandenen Zutaten, Handwerksstufe und tatsächlichen Inventarrezepten. Ein Vorgang verbraucht alle Zutaten und erzeugt das Ergebnis atomar.
- Zentraler Dorfkatalog mit fünf Gegenständen und zwei Rezepten. Der C++-Katalog wird aus der JSON-Quelle erzeugt und auf Synchronität geprüft. ItemData-Pakete entstehen erst durch den Unreal-Editor.
- Save-Format 5: Händlergold und ItemID-Bestände unter MapName/MerchantID. Fehlende Belege auf der gespeicherten Karte, doppelte Händler-IDs und ungültige Bestände verhindern den Restore. V1–V4 setzen Händler auf ihren initialen Bestand zurück. Neu besuchte Karten erhalten ihren eigenen initialen Bestand.
- Kauf-/Verkauf-/Craftingpläne prüfen Zahlenbereiche, Ressourcen und Katalogmehrdeutigkeiten vor Änderungen. Rückrufe sehen erst die gemeinsam aktualisierten Konten und Inventare. Laufende Transaktionen verhindern Speichern/Laden und rekursive Transaktionen.
- Ausrüstung wird vor Transaktionsrückrufen mit dem neuen Inventar abgeglichen. Die letzte verkaufte oder verarbeitete Waffe kann keinen Bonus zurücklassen.
- Erfolgreiches Wiederherstellen schließt offene Menüs zentral vor den Restore-Rückrufen, auch beim F9-Schnellladen; fehlgeschlagene Restores erhalten den bestehenden Kontext. Controller ohne lokalen Spieler erzeugen kein UMG-Widget.
- HUD-Aktionen tragen eine Kontextgeneration. Alte Händler-, Inventar- und Magiebuttons können nach Schließen, Neuaufbau oder Kontextwechsel keine Aktionen durchführen. Händler/Werkbankmenüs schließen bei Tod, Entfernung oder zerstörtem Ziel.
- GitHub-Review-Korrekturen übernommen: LFS-Checkout in CI, explizite World-/Actor-Includes, auffindbare Python-Testpackage und verständliche Fehlerausgabe. Die CI führt nun den V79-Prüfer aus.
- ZIP-Pipeline prüft Version, grüne Offline-Ergebnisse, unveränderte Quellinputs, materialisierte LFS-Dateien sowie CRC und SHA256 sämtlicher Einträge. Ein Zwischenarchiv und ein abschließendes Entwicklungsarchiv werden getrennt gesichert.

## Herkunft

Die Dorfökonomie-Daten und C++-/Python-Implementierungen wurden für dieses Projekt erstellt. Sie benötigen keine zusätzlich gekauften Assets. Vorhandene originale Blender-/Animations-/Audioquellen und ihre Lizenznachweise bleiben enthalten. Edda und Werkbank verwenden in der Editorpipeline vorläufig Engine-Grundformen; Edda hat noch kein eigenes Charaktermodell, Rig oder Tagesablaufsystem.

## Noch nicht nachgewiesen

Unreal Engine 5.6, Unreal Build Tool und ein Windows-Host fehlen in dieser Umgebung. Deshalb sind C++-/UHT-Kompilierung, Assetimport, Kartenproduktion, Navigation, UMG-Bedienung, native Automation, Speichern im echten Spiel und Win64-Veröffentlichung **blockiert**. Portable Tests bestätigen nur die tatsächlich ausgeführten Regeln; Python-Prüfungen enthalten auch Quellverträge und ersetzen keine Engine-Tests.

Es gibt weiterhin keine echte `.umap`, `.uasset` oder Win64-EXE. Der Dorfablauf ist nicht als vollständig spielbar abgenommen. Die Kampagne, 30 fertige Levels, mindestens 20 spielbare Nebenquests, drei Enden, produktionsreife Effekte, Audiointegration und Performance-Messungen bleiben offen. FPS wurden nicht gemessen.

## Nächster ausführbarer Engine-Schritt

Die mitgelieferte V79-Windows-Pipeline auf einem UE-5.6-Rechner ausführen. Anschließend Dornfeld manuell testen: Mara-Auftrag, drei Scherben, zwei Wächter, Truhe, 90 Gold/180 XP; Zutaten aufnehmen, beide Rezepte herstellen, Gegenstände nutzen/ausrüsten, Edda kaufen/verkaufen, mehrere Saves/Loads inklusive älterem Save und finaler Waffenverkauf. Jeden Status erst nach realem Test markieren.
