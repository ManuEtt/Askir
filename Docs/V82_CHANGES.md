# V82 – Zielerfassung und gerichteter Kampf

Ausgangsbasis: V81 `b243a5e`. Vor Änderungen wurde die vollständige V81-ZIP
nach `/workspace/backups/AskirRPG_V81_PreV82.zip` gesichert. V81 ist auf GitHub
in Draft-PR #2 verfügbar; V80 liegt bereits in main.

## Implementiert, Engine-Ausführung offen

- Neue TargetLock-Komponente an der nativen Spielfigur. Enhanced Input bindet
  mittlere Maustaste und R. Sicht, Kamerakegel, Entfernung, Feindseligkeit und
  aktiver Begegnungszustand begrenzen Erwerb und Zielwechsel.
- Kamera und Figur folgen dem Ziel, Bewegungsorientierungsflags werden während
  der Erfassung gesichert und beim Lösen vollständig wiederhergestellt.
- Gemeinsame Zielrichtung für Nahkampf und Magie; keine Projektil-Zielverfolgung.
  Neue Zielrichtungen benötigen sofort freie Augen- und Wirkungsursprung-Sicht.
  Kurze Sichtunterbrechungen erhalten die Auswahl höchstens 0,35 Sekunden.
- Native UMG-Gegneranzeige mit Name, LP und DPI-korrekter Körpermarkierung.
  Menü, Tod, Reise und ungültige Ziele blenden sie aus.
- Gemeinsame Kampfberechtigung für Nahkampf, offensive Magie und Zielerfassung.
  NPC-KI prüft sie vor dem Angriff und erneut beim eigenen verzögerten Treffer.
- Frontblock statt Rundumschutz. Ein nativer Blockversuch schließt einen zweiten
  Legacy-Guard-Versuch für denselben Treffer auch bei Ausdauermangel aus.
  Unbekannte oder ungültige Richtungen und wahrer Schaden gewähren keinen Block.
- Projektile melden ihre Flugrichtung an die Trefferlogik. Eine inzwischen
  veränderte Position des Zaubernden verfälscht die Blockrichtung dadurch nicht.
- Der 70-cm-Startversatz eines Zaubers wird auf Hindernisse geprüft. Eine
  blockierte Erzeugung verbraucht weder Mana noch Abklingzeit.
- Nahkampf-Hitfeedback wird nur nach tatsächlichem Lebensverlust ausgelöst.
- Zielreferenzen sind transient. Menü, Tod, Laden und Reise löschen sie über
  die bestehende Aktionsrücknahme. Speicherformat 5 bleibt bestehen.

## Prüfungen und Einordnung

Neue portable C++-Tests prüfen unmittelbar verwendete Produktionsregeln für
Kampfberechtigung, Frontkegel, Zielgeometrie, Ranking, Zyklus und Sichtfristen.
Native Tests ergänzen reale Kollisions-/Akteur-/Wiederherstellungsprüfungen;
sie sind ohne Engine nicht ausgeführt.

Der frühere Python-Stringvergleich gegen die inzwischen entfernte doppelte
Magie-Zielprüfung entfällt. Die neue gemeinsame Produktionsregel wird tatsächlich
als C++ ausgeführt; native Tests prüfen ihre Unreal-Anbindung nach verfügbarem
Engine-Build. Die Dorfmissions-Fixture kennzeichnet ihre Wächter nun ausdrücklich
als feindlich, wie bereits die eigentliche Editorpipeline.

V82 fügt keine fertigen Unreal-Levels oder importierten Binärassets hinzu.
Weltaufbau, Rendering, Animationen im seitlichen Lauf, Bedienung und Performance
bleiben Gegenstand der Engine-Abnahme.
