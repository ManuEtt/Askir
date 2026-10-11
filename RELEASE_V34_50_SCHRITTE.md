# Askir RPG V34 — Entwicklungspaket 701–750

## Implementiert in dieser Version

- Kill-Credit-Zeitfenster (`KillCreditWindowSeconds`, standardmäßig 8 Sekunden) im Combat Component.
- `GetLastDamageCauser()` liefert nur einen Verursacher innerhalb des gültigen Zeitfensters.
- `ClearDamageAttribution()` für explizites Zurücksetzen bei Umweltereignissen.
- Schadenszeitstempel wird nur bei tatsächlich angewendetem positiven Schaden gesetzt.
- Reset der Lebenspunkte löscht die Kill-Zuordnung.
- RestoreHealth löscht Kill-Zuordnung und sendet **kein** OnDeath mehr; verhindert doppelte Loot-/Quest-Ereignisse beim Laden.
- Editor-Automationstest `Askir.Combat.RestoreDoesNotReplayDeath` als Quellcode ergänzt.

## 50 konkrete Themen im Arbeitsblock (nicht alle implementiert)

701. Kill-Credit-Zeitfenster definieren **[offen]**
702. Letzten Treffer mit Weltzeit erfassen **[offen]**
703. Ungültige Schadensverursacher verwerfen **[offen]**
704. Attribution beim Reset löschen **[offen]**
705. Attribution beim Laden löschen **[offen]**
706. Keine Todes-Delegates beim Laden auslösen **[offen]**
707. Explizites Löschen für Umwelt-Tode **[offen]**
708. Kill-Credit-Grenzfälle dokumentieren **[offen]**
709. Test für tote SaveGame-Akteure ergänzen **[offen]**
710. Kompilierung im Unreal Editor durchführen **[offen]**
711. Kill-Credit über Projectile Owner prüfen **[offen]**
712. Instigator-Ketten über mehrere Besitzer absichern **[offen]**
713. PlayerController als Killer prüfen **[offen]**
714. Umweltschaden-Klassifikation ergänzen **[offen]**
715. Gegner-AI-Kill gegen Spieler testen **[offen]**
716. Mehrspieler-Kill-Attribution vorbereiten **[offen]**
717. Mehrfachtreffer-Ketten testen **[offen]**
718. Todesereignisse idempotent halten **[offen]**
719. Kill-Attribution zeitlich testen **[offen]**
720. Gameplay-Log für Kill-Credit hinzufügen **[offen]**
721. Quest-HUD UMG Widget erzeugen **[offen]**
722. OnObjectiveChanged im HUD binden **[offen]**
723. Quest-Texte lokalisieren **[offen]**
724. Questziel-Zähler anzeigen **[offen]**
725. Questjournal synchronisieren **[offen]**
726. Laden der Questanzeige testen **[offen]**
727. Zielwechsel-Animation gestalten **[offen]**
728. Questmarker im Level anbinden **[offen]**
729. Mission-Abschlussmeldung gestalten **[offen]**
730. HUD-Automationstest entwickeln **[offen]**
731. Einmalige Belohnungen validieren **[offen]**
732. Gold und XP atomar vergeben **[offen]**
733. Amulett-Item-Asset definieren **[offen]**
734. Inventarkapazität bei Belohnung prüfen **[offen]**
735. Quest-Belohnung bei vollem Inventar behandeln **[offen]**
736. Generische Quest und Dornfeld-Rewards abgleichen **[offen]**
737. Quest-IDs zentral konfigurieren **[offen]**
738. SaveGame-Version migrieren **[offen]**
739. Belohnungs-Regressionstest ergänzen **[offen]**
740. Mehrfach-Laden-Speichern testen **[offen]**
741. Dornfeld-Level als .umap erzeugen **[offen]**
742. Mara-NPC und Dialog im Editor erstellen **[offen]**
743. Drei Scherben-Actors platzieren **[offen]**
744. Zwei Wächter mit Kill-Relay platzieren **[offen]**
745. Kellertruhe konfigurieren **[offen]**
746. Quest-HUD im PlayerController aktivieren **[offen]**
747. Level-Übergänge prüfen **[offen]**
748. End-to-End-Mission im PIE testen **[offen]**
749. Windows-Build paketieren **[offen]**
750. Spiel-Build außerhalb des Editors prüfen **[offen]**

## Einschränkungen
Kein Unreal Engine 5.6 Editor, keine C++-Toolchain für Unreal und keine Asset-Authoring-Pipeline in dieser Laufzeit. Es wurden keine `.umap`, `.uasset` oder Windows-EXE erzeugt. Das ZIP ist ein vollständiges **Quellprojekt**, kein getestetes spielbares Spiel. Unreal-Tests wurden geschrieben, nicht ausgeführt.
