# ASKIR RPG · V79 Entwicklungsstand

Unreal Engine 5.6, C++ und Third Person, Zielplattform Windows 64 Bit.

**Quellprojekt mit originalen Assetquellen. Unreal-Kompilierung, Spielbarkeit und Windows-Build sind noch nicht nachgewiesen: Hier fehlen Unreal Engine 5.6 und ein Windows-Host. Das Archiv enthält keine `.umap`, `.uasset` oder Spiel-EXE.**

V79 verbindet Dorfhändlerin Edda und eine Werkbank mit nativen UMG-Menüs. Kaufen und Verkaufen prüfen den tatsächlichen Bestand, Spielerinventar und beide Geldkonten; die Werkbank verbraucht Zutaten und erzeugt Inventargegenstände. Zwei Dornfeldrezepte sind vorbereitet: Heiltrank und Jagdschwert. Save-Format 5 sichert Händlergold und Warenbestand über stabile IDs pro Karte; ältere Saves werden migriert. Veraltete Menübuttons können nach Kontextwechsel keine Transaktionen auslösen. Der Verkauf der letzten ausgerüsteten Waffe entfernt ihren Schadensbonus.

Diese Unreal-Integrationen liegen als unkompilierter C++-Code und Editor-Pipeline vor. Die V79-Pipeline erzeugt im echten Editor fünf Gegenstands-DataAssets, Händler, Werkbank und persistente Zutatenpickups und überprüft die gespeicherte Dorfkarte, bevor sie den Startlevel ändert. Ihre Ausführung ist hier blockiert.

Die V78-Magie mit 24 kanonischen Zaubern, die originalen Rigs und jeweils zwölf Animationsclips für drei Figuren sowie neun WAV-Quellen sind erhalten. Auch die V77-Dorfmission „Das Flüstern im Brunnen“ mit 90 Gold/180 XP, Enhanced Input, Questjournal und versionierten Save-Bänken ist erhalten. Ihr End-to-End-Gameplay wurde hier nicht ausgeführt.

Offline prüfen:

```bash
python -m pip install -r requirements-dev.txt
python Tools/verify_v79.py
```

Der Prüfer führt Python-Regressionen, echte portable Produktionsregeln mit g++/UBSan und Assetprüfungen aus. `Reports/V79_verification.json` trennt deren Ergebnisse von blockierten Unreal-/Windows-Prüfungen. ZIP-Erstellung verlangt dieselben geprüften Quelldateien und verifiziert CRC sowie alle Dateihashes.

Windows mit UE 5.6, Visual-Studio-C++-Buildwerkzeugen/Windows SDK und Python 3.10+:

```powershell
.\Scripts\Build-AskirV79.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.6" -OutputDirectory "C:\AskirBuilds\V79"
```

Der Ausgabeordner muss neu sein und außerhalb des Projekts liegen. Die Pipeline sichert das Projekt, kompiliert den Editor, importiert Charaktere und Audio, erzeugt die Dorfkarte, führt native Automation aus, erstellt das Win64-Paket und prüft dessen Startmarker. Alle Schritte stoppen bei fehlendem Nachweis. Ein Starttest ersetzt kein manuelles Durchspielen.

Details: [V79-Änderungen und Grenzen](Docs/V79_CHANGES.md), [Dorfpipeline](Docs/V79_DORNFELD_ECONOMY_PIPELINE.md), [V76-Bestandsaudit](Docs/V77_TECHNICAL_AUDIT.md).

Die vollständige Kampagne, mindestens 30 spielbare Levels und ein getesteter Windows-Build bleiben offen. GitHub-Schreibzugriff wurde im vorherigen Repository-Review mit HTTP 403 verweigert; diese Version wird lokal und als ZIP gesichert.
