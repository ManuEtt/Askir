# V82: Unreal-Build und Nachweisgrenzen

V82 erweitert die V81-Pipeline um den Bereitschaftsnachweis der Zielerfassung. Die bestehenden Weltpakete `/Game/Askir/Maps/Dornfeld_V80` und `/Game/Askir/Maps/Dornfelder_Umland_V80` bleiben erhalten. Es werden keine umbenannten oder leeren V82-Karten als neue Levels gezählt.

Auf einem Windows-Rechner mit Unreal Engine 5.6 und Python 3.10 oder neuer:

```powershell
.\Scripts\Build-AskirV82.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' -OutputDirectory 'D:\AskirBuilds\V82_01' -CheckOnly
.\Scripts\Build-AskirV82.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.6' -OutputDirectory 'D:\AskirBuilds\V82_02'
```

Jeder Lauf benötigt einen neuen Ausgabeordner außerhalb des Projekts. Ohne `-CheckOnly` erstellt die Pipeline vor Editor-Änderungen eine Sicherung unter `Backup/Askir_pre_v82_generation`, kompiliert den Editor, erzeugt oder lädt die zwei echten V80-Welten, prüft deren Editor-Berichte, führt alle registrierten `Askir.*`-Automation-Tests in beiden Welten aus und erstellt anschließend ein Win64-Development-Paket. `-UseExistingMap` prüft vorhandene V80-Welten ohne erneute Platzierung der Inhalte.

Der paketierte Spielstart wird getrennt für das Startdorf und das Umland geprüft. Der Dorfstart verwendet die wirkliche `GameDefaultMap`; das Umland wird über seinen expliziten Paketpfad gestartet. Jeder aktuelle Start muss diese Nachweise enthalten:

```text
ASKIR_V80_RUNTIME_READY map=Dornfeld_V80 player=AskirCharacter_0
ASKIR_V82_TARGETING_READY map=Dornfeld_V80 player=AskirCharacter_0 component=AskirTargetLockComponent
ASKIR_V81_FRONTEND_READY map=Dornfeld_V80 view=Title
```

Die Reihenfolge von Titel- und Zielerfassungsmarker ist frei. Beide müssen nach der letzten wiederhergestellten Spielerwelt erscheinen. Der Zielerfassungsmarker muss dieselbe Map und denselben kontrollierten Spieler wie der Weltmarker melden. Die native Spielinitialisierung darf ihn erst nach Prüfung der tatsächlich vorhandenen Komponente und ihres Besitzers ausgeben. Für das Umland gelten dieselben Regeln mit `Dornfelder_Umland_V80`.

Vorhandene Startup-Logs, fehlende oder unvollständige Marker, fremde Maps, andere Spieleridentitäten und abweichende Komponententypen führen zum Abbruch. Ein erfolgreicher Build-Prozess allein genügt nicht. Fehlende Engine oder ein fehlender Windows-Host bleiben `Blockiert`; nachfolgende Schritte bleiben `Nicht ausgeführt`.

Die Startup-Prüfung läuft mit `-NullRHI -nosound`. Sie belegt weder sichtbare Darstellung noch Eingaben, Zielauswahl, Kamera-Nachführung, Kampf oder den tatsächlichen Wechsel zwischen den Welten. `game_acceptance`, `frontend_acceptance` und `targeting_acceptance` bleiben deshalb auch bei erfolgreicher technischer Pipeline `Nicht ausgeführt`. Die Offline-Tests verwenden ausdrücklich synthetische Logs und prüfen ausschließlich diese Nachweisgrenzen.
