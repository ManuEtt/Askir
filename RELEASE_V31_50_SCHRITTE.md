# Askir RPG V31 – Schritte 551–600

Die 50 Punkte sind Entwicklungs-, Integrations- und Prüfpositionen. Editor- und Laufzeittests sind **noch offen**.

## Architektur

551. V30 als Basis übernommen
552. Mission als eigene ActorComponent
553. Mission mit BlueprintType exponiert
554. Stufen als UENUM definiert
555. Save-Snapshot als USTRUCT angelegt
556. Stage-Event-Delegate definiert
557. Missionskomponente im Spieler registriert
558. Editor-Event-Actor ergänzt
559. Unreal-Interface für Interaktion genutzt
560. Konfigurierbare Reichweite eingebaut

## Questablauf

561. Startgespräch implementiert
562. Drei einzigartige Scherben erfasst
563. Doppelte Scherben verworfen
564. Scherbenstufe automatisch beendet
565. Zwei einzigartige Wächter erfasst
566. Doppelte Wächter verworfen
567. Wächterstufe automatisch beendet
568. Truhenereignis implementiert
569. Rückkehr zur Ältesten implementiert
570. Abschlussstatus eingeführt

## Belohnung und Robustheit

571. Goldbelohnung 90 integriert
572. XP-Belohnung 180 integriert
573. Belohnung nur einmal auszahlbar
574. Goldüberlauf vorab geprüft
575. Fehlende Wallet abgefangen
576. Fehlende XP-Komponente abgefangen
577. Leere Ereignis-IDs abgewiesen
578. Stufenfremde Ereignisse abgewiesen
579. Entfernung bei Ereignissen geprüft
580. Wächtertod nicht durch Interaktion auslösbar

## Speichern und Laden

581. Missionssnapshot in USaveGame ergänzt
582. Missionsdaten beim Speichern erfasst
583. Missionsdaten beim Laden wiederhergestellt
584. Scherben-IDs gespeichert
585. Wächter-IDs gespeichert
586. Truhen-ID gespeichert
587. Belohnungsstatus gespeichert
588. Doppelte IDs bei Restore bereinigt
589. Ungültige Completed-Zustände korrigiert
590. Blueprint-Stage-Event nach Restore gesendet

## Dokumentation und QA

591. Dorf-Editor-Anleitung erstellt
592. Mara-Setup dokumentiert
593. Scherben-Setup dokumentiert
594. Gegner-Death-Event dokumentiert
595. Truhen-Setup dokumentiert
596. HUD-Anbindung dokumentiert
597. Save/Load-Prüfszenario dokumentiert
598. Wiederholungsprüfungen dokumentiert
599. Statische Quellcodeprüfung durchgeführt
600. ZIP-CRC-Prüfung durchgeführt
