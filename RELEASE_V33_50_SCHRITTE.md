# Askir RPG V33 – Schritte 651–700

651. V32-Basis vollständig übernommen
652. Kampfschaden nach tatsächlichem HP-Verlust bewertet
653. Letzten Schadensverursacher im Combat-Component gespeichert
654. Schadensverursacher per Blueprint getter verfügbar
655. Weak-Pointer verhindert harte Actor-Referenzen
656. Verursacher nur bei positivem Schaden aktualisiert
657. Ausweich-Immunität ändert Killer nicht
658. Geblockter Nullschaden ändert Killer nicht
659. Verursacher beim Gesundheitsreset zurückgesetzt
660. Todes-Delegate bleibt kompatibel
661. Guardian-Relay validiert Combat-Component
662. Guardian-Relay validiert Missionsspieler
663. Guardian-Relay lehnt unbekannte Killer ab
664. Guardian-Relay akzeptiert direkten Spielertreffer
665. Guardian-Relay akzeptiert vom Spieler besessene Actoren
666. Guardian-Relay akzeptiert Projektile mit Spieler-Instigator
667. Guardian-Relay lehnt NPC-Kills ab
668. Guardian-Relay lehnt Umwelttode ohne Quelle ab
669. Guardian-Relay nutzt bestehende Mission-IDs
670. Guardian-Relay nutzt vorhandene Duplikatsperre
671. Quest-HUD erhält ObjectiveChanged-Delegate
672. Delegate überträgt aktuellen FText
673. Delegate überträgt Missionsphase
674. Phasenwechsel aktualisiert Quest-HUD
675. Erste Scherbe aktualisiert Quest-HUD
676. Zweite Scherbe aktualisiert Quest-HUD
677. Dritte Scherbe löst Phasenwechsel aus
678. Erster Wächter aktualisiert Quest-HUD
679. Zweiter Wächter löst Phasenwechsel aus
680. Truhenöffnung aktualisiert Quest-HUD
681. Queststart aktualisiert Quest-HUD
682. Questabschluss aktualisiert Quest-HUD
683. Savegame-Restore aktualisiert Quest-HUD
684. Bestehender StageChanged-Delegate erhalten
685. Bestehende Goldbelohnung erhalten
686. Bestehende XP-Belohnung erhalten
687. Belohnungsschutz beibehalten
688. Persistente Scherben-IDs erhalten
689. Persistente Gegner-IDs erhalten
690. Persistente Truhen-ID erhalten
691. Automatisierten Duplikattest nach Laden erweitert
692. Automatisierten Truhen-Duplikattest ergänzt
693. Editor-Setup für Projectile-Instigator dokumentiert
694. Editor-Setup für HUD-Bindung dokumentiert
695. Kill-Attribution-Grenzen dokumentiert
696. Regressionstestliste erstellt
697. Projektdateien und Content-Daten beibehalten
698. Dateireferenzen statisch geprüft
699. ZIP-Integritätsprüfung vorgesehen
700. Unreal-Kompilierung als offener QA-Schritt ausgewiesen

## Grenzen
Nicht mit Unreal Engine kompiliert oder im Editor getestet. Der letzte positive Schadensverursacher ist eine pragmatische Attribution, keine Netzwerk-/Assist-Logik.
