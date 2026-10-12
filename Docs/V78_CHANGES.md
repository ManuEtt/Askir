# V78 – Magie und animierte Charakterquellen

**Entwicklungsstand, keine freigegebene Spielversion.** Unreal 5.6 und Windows sind in dieser Umgebung weiterhin nicht verfügbar. Es wurden keine `.umap`, `.uasset` oder Windows-Executables erzeugt. Die umfangreiche Kampagne und die 30 Level bleiben offen.

## Tatsächliche Weiterentwicklung

- Die 24 Einträge aus `Production/Magic/spells.json` erzeugen einen überprüfbaren, tatsächlich vom Laufzeitcode verwendeten C++-Katalog. Sechs Schulen unterstützen ihre vorhandenen Projektil-, Schild-, Klingenbogen- und Flächentypen. Die Licht-Nova heilt zusätzlich den Zaubernden; dieser Laufzeitzusatz wird ausdrücklich dokumentiert und verändert die kanonischen Quelldaten nicht.
- Projektile führen einen kontinuierlichen Sweep aus und ordnen Treffer dem Instigator zu. Klingenbogen und Fläche prüfen Reichweite, Blickwinkel beziehungsweise Radius, Sichtlinie und einzigartige Ziele. Zivilisten, tote und inaktive Questgegner werden geschützt. Schilde absorbieren positive Schäden und lassen TrueDamage passieren. Diese Engine-Integrationen sind unkompiliert.
- Mana, Auswahl, verbleibende Abklingzeiten und Schildkapazität/-dauer sind in Save-Format 4 integriert. Versionen 1–3 laden mit vollständigem Standardmana und ohne neue magische Zustände. Ungültige Magiedaten werden vor Zustandsänderungen verworfen; Projektile der alten Timeline werden bei Wiederherstellung entfernt.
- F wirkt den ausgewählten Zauber, Mausrad wechselt und K öffnet die Magieauswahl. Das native UMG zeigt Mana, Schild und Abklingzeit. Geöffnete Menüs sperren jetzt auch Sprung, Sprint, Dodge, Block und Zaubern und brechen laufende Spieleraktionen ab. Die XP-Anzeige verwendet die vollständige Levelschwelle.
- Drei originale Humanoidquellen für Hero, Guard und Raider wurden in Blender erzeugt. Sie enthalten tatsächliche Skinninggewichte und zwölf Clips. Der FBX-Importer und die native Playback-Komponente prüfen gemeinsamen Skeleton und vorhandene Pakete vor dem Wechsel vom bisherigen Stellvertreter. Einzelheiten und tatsächliche Blender-Prüfungen stehen in den Characterberichten; Unreal-Import ist nicht ausgeführt.
- Zwei neue originale WAV-Zauberklänge ergänzen die sieben V77-Quellen. Format, Dauer und Aussteuerung wurden tatsächlich geprüft.

## Prüfungen und ZIP-Zwischenstände

`python Tools/verify_v78.py` prüft Struktur, Python-Regressionen, kanonischen Katalog, Audio, die realen FBX-Eingaben und drei mit UBSan kompilierte Produktionsregeltests. Die Magieregeln liegen im tatsächlich verwendeten `AskirMagicRules.h`; die Tests sind kein unabhängiges Nachbau-Modell. Berichte unterscheiden Offline-Erfolg von Unreal-Blockaden.

`Tools/package_checkpoint.py` erstellt ZIP-Zwischenstände aus materialisierten Projektdateien. Danach prüft es CRC, Dateiliste und den SHA-256 jedes Eintrags gegen die Quellen. Versionen liegen außerhalb des Projekts und überschreiben den V77-Stand nicht.

Auf einem Windows-UE5.6-Rechner wird `Scripts/Build-AskirV78.ps1` wie in der README beschrieben gestartet. Es kompiliert den Editor, importiert die echten FBX-/WAV-Quellen, erzeugt eine neue `Dornfeld_V78`-Karte, prüft Save/Reload, Referenzen und Navigationspfade, führt native Automation aus und baut/startet Win64. Erst tatsächliche Editorprüfungen ändern den Standardstartlevel. Die Pipeline wurde hier nur bis zur konkreten Werkzeugblockade ausgeführt. Visuelle Animationen, Magie, UMG und Dorfmission benötigen weiterhin Engine- und manuelle Gameplaytests.

GitHub-Schreibzugriff wurde in V77 durch HTTP 403 abgelehnt. Diese Iteration nutzt lokale Sicherungen und ZIP; kein neuer Remoteupload wird behauptet.
