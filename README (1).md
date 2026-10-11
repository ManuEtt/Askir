# Askir RPG – Unreal Engine 5.6

Fantasy-RPG-Projekt mit C++-Gameplay-Prototypen, KI, Handel, Quests, Speichern und 3D-Modellen.

**V20:** 50 dokumentierte Arbeitsschritte zu Robustheit, Release-Prozess und Tests. Siehe [Releasebericht](RELEASE_V20_50_SCHRITTE.md) und [GitHub-Import](GITHUB_IMPORT.md).

**Unreal-Status:** Projektquellen vorhanden; keine erfolgreiche Unreal-5.6-Kompilierung oder PIE-Laufzeitprüfung bestätigt. Charaktermodelle sind noch keine fertigen animierten Produktionsassets.

```bash
python Tools/validate_release.py
python -m unittest discover -s Tests -v
```

Askir RPG - UE 5.6 C++ foundation

Included:
player movement, camera, sprint, jump, inventory, item data, pickups,
interaction interface, health/damage/resistance, melee combat, NPC base,
hostile AI/navigation foundation, world time, simulation subsystem and SaveGame.

This package was generated as source. It has NOT been compiled in an actual
Unreal Engine 5.6 installation here. Generate project files and compile in
the local UE 5.6 environment before continuing.

The architecture is deliberately modular for later expansion:
needs/schedules, perception, relationships, factions, economy, quests,
magic, injuries, world events and persistent simulation.
## Erweiterte emergente Systeme

Die Projektbasis enthält jetzt zusätzlich Bedürfnisse, Tagesabläufe, Beziehungen,
Fraktionen, NPC-Wahrnehmung/Gedächtnis, Dialogdaten, Quests, dynamische Marktpreise,
globale Welt-Events und einen fortgeschrittenen NPC-Komponentenverbund.

Die Systeme sind bewusst datengetrieben und modular, damit daraus emergentes Verhalten
entstehen kann: Bedürfnisse verändern Entscheidungen, Erinnerungen beeinflussen soziale
Beziehungen, Fraktionen verändern Reaktionen und Welt-Events verändern die Simulation.

Wichtig: Dieses Paket ist Quellcode. Es wurde hier nicht gegen eine installierte Unreal
Engine 5.6 kompiliert. Die lokale UE-5.6-Toolchain muss die Build-Prüfung übernehmen.

## Kampfsystem – aktueller lokaler Entwicklungsstand

- Linke Maustaste: leichter Angriff; rechte Maustaste: schwerer Angriff.
- Q halten: Blocken (verringert Schaden, kostet Ausdauer).
- Linke Alt-Taste: Ausweichen per LaunchCharacter (noch ohne Unverwundbarkeitsfenster).
- Ausdauer regeneriert nach einer Pause. Die Werte sind am Charakter editierbar.
- Nahkampf-Sweep startet an der Spielfigur und prüft Visibility-Kollision; Gegner müssen für Treffer entsprechend konfiguriert sein.
- Gegner-Schaden wird über ReceiveAskirDamage verarbeitet, sodass Blocken berücksichtigt wird.

Hinweis: Die Änderungen sind Quellcode und wurden noch nicht in UE 5.6 kompiliert oder spielgetestet.

## Gegner-Kampfstile (Entwicklungsstand)
NPCs bieten jetzt drei im Editor auswählbare Nahkampfprofile: Balanced (Standard), Bruiser (langsamer, stärker) und Skirmisher (schneller, geringerer Schaden). Multiplikatoren sind pro NPC einstellbar. Die Umsetzung wurde noch nicht in Unreal Engine kompiliert oder getestet.

## Trefferreaktionen (lokale Entwicklung)
Feindliche NPCs können bei ausreichend starkem Schaden kurzzeitig ins Taumeln geraten.
Schwelle, Dauer und Schutzzeit gegen wiederholtes Taumeln sind im Editor einstellbar.
Während des Taumelns pausieren ihre KI-Entscheidungen; ein Blueprint-Event `OnStaggered`
steht für spätere Animationen und Effekte zur Verfügung. Noch nicht in UE kompiliert.

## Enemy attack windup (development)
Enemy melee attacks now telegraph before applying damage. At impact, range and line of sight are rechecked, so retreating behind cover or dodging away can avoid a hit. Stagger and death interrupt pending attacks. Optional Blueprint events `OnAttackTelegraphed` and `OnAttackLanded` provide animation/VFX hooks. This code has not been compiled in Unreal Engine.

## Combat feedback hooks (Blueprint)

The C++ character exposes optional BlueprintImplementableEvent hooks: `OnAttackStarted`, `OnAttackHit`, `OnAttackMissed`, `OnDodgeStarted`, and `OnBlockStateChanged`. NPCs expose `OnAttackTelegraphed`, `OnAttackLanded`, `OnAttackAvoided`, `OnAttackInterrupted`, and `OnStaggered`. Attach animation montages, sound cues, camera feedback and VFX in Blueprint child classes. No animation assets are bundled and these events do not imply a completed animation integration.

## Save/load integration (development)
The save slot now serializes player location, rotation, health, stamina and simulation world state. Loading restores these values and broadcasts the simulation update event. Inventory and quest persistence are not yet connected; UE 5.6 compilation and runtime validation remain outstanding.

## Save/Load development update

Player inventory entries now persist using soft asset paths and quantities; quest states and active objective progress are serialized alongside position, health, stamina and world state. Missing inventory assets abort load rather than dropping items. The quest component is attached to the player character. Unreal Engine compilation and runtime testing are still pending.

## Quest objective integration (October 2026)

- A pickup advances an active quest objective whose `ObjectiveID` matches the picked-up `UAskirItemData::ItemID`.
- An NPC with `DefeatObjectiveID` advances the corresponding objective for the actor recorded as its last damage causer on death.
- `CompleteQuest` now requires an active quest with all objectives complete; `AreObjectivesComplete()` is available to Blueprints.
- Quest data assets and an NPC quest-giver interaction are still needed for a complete playable quest loop.
- Not compiled or playtested in Unreal Engine.

## Erweiterung 11.10.2026: Charakterentwicklung (10 Themen)

1. Erfahrungspunkte vergeben (AwardExperience)
2. Levelaufstieg und steigende XP-Schwellen
3. Attributpunkte vergeben
4. Fünf Charakterattribute (Stärke, Geschick, Vitalität, Intellekt, Wille)
5. Fertigkeiten trainieren und Ranggrenzen
6. Talente/Perks mit Level- und Vorbedingung
7. Handwerksrezepte erlernen und abfragen
8. Charakterbezogener Fraktionsruf (separat vom vorhandenen Fraktionssystem)
9. Attributbonus und Fertigkeits-Erfolgschance berechnen
10. Snapshot-/Restore-API als Grundlage für spätere Persistenz

Neu: `UAskirProgressionComponent` wird am Spielercharakter erzeugt.
Die bestehende SaveGame-Integration speichert diesen Snapshot **noch nicht**.
Dies ist Quellcode und wurde **nicht in Unreal Engine kompiliert oder getestet**.
