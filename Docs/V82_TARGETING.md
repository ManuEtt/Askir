# V82 – Zielerfassung

Die native Zielerfassung verwendet dieselben tatsächlich ausgeführten Geometrie-, Ranking-, Zyklus- und Zeitregeln wie der portable C++-Test. Die Unreal-Komponente selbst benötigt Unreal Engine 5.6; ihre Registrierung im Projekt ersetzt keinen Engine-Test.

## Verhalten im Quellcode

- Erwerb eines lebenden, aktiven, feindlichen NPC innerhalb von 1400 cm und einem Kamerakegel von ±75°. Zivile, verborgene, deaktivierte, tote und verdeckte NPCs scheiden aus. `AskirCombatTargetPolicy::CanDamage` legt die gemeinsame Kampfberechtigung fest.
- Sichtlinie wird vom Augenpunkt der Spielfigur geprüft. Die Auswahl gewichtet Kamerazentrum und Entfernung. Gleiche Werte werden anhand des Actor-Pfades deterministisch aufgelöst.
- Richtungswechsel wählt den nächsten sichtbaren Kandidaten rechts beziehungsweise links um die Kameraachse. Ohne zweiten Kandidaten bleibt das Ziel erhalten.
- Das Ziel bleibt bis 1800 cm erhalten. Deckung erhält die Auswahl höchstens 0,35 Sekunden. Neue Zielrichtungen für Nahkampf und Zauber erfordern sofort freie Augen- und Wirkungsursprung-Sichtlinien; Projektile erhalten keine nachträgliche Zielverfolgung.
- Während der Erfassung richtet sich die Figur zum Ziel aus und kann seitlich laufen. Die Kamera nähert sich weich der Blickrichtung. Beim Lösen werden die ursprünglichen Rotationsflags wiederhergestellt.
- Tod, deaktivierte Feindseligkeit, deaktivierter Encounter, ungültiger Actor, andere Welt, Reichweitenverlust oder gesperrte Gameplay-Eingabe lösen die Erfassung. Menü-, Reise- und Ladeintegration löscht sie über `CancelGameplayActions`. Zielreferenzen werden nicht gespeichert.

## Nachweise

`Tests/cpp/test_targeting_rules_v82.cpp` prüft die produktiven portablen Regeln mit endlichen/ungültigen Vektoren, Überlaufgrenzen, Entfernungs- und Kegelgrenzen, vollständigen Winkelrastern, Rangfolge, allen Paaren von 72 Richtungen in beiden Zyklusrichtungen und verdeckten Zielen an der Zeitgrenze. Das Ergebnis steht im V82-Verifikationsbericht.

Vier native Tests sind registriert: `AcquisitionPolicyAndRotationRestore`, `DirectionalCycleAndSingleCandidate`, `CoverBlocksAimImmediatelyAndReleasesAfterGrace`, `StateChangesReleaseWithoutStaleAim`. Sie verwenden gespawnte Figuren, Controller und echte Kollisionskörper in einer Unreal-Testwelt. Status ohne Engine: **Blockiert / nicht ausgeführt**.

Für die Engine-Abnahme sind insbesondere Kameraverhalten, seitliche Bewegungen mit den bestehenden Animationen, Sichtlinien um Mauerecken, die Übergänge zu Menü/Laden/Reisen und der vollständige Dorfmissionskampf manuell zu prüfen. Die Änderungen erzeugen keine Unreal-Maps und liefern keinen Windows-Build.
