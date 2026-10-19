# V81: Native Hauptmenü und Einstellungen

Status: C++-Implementierung und portable Regeln vorhanden. Unreal-Kompilierung,
UMG-Darstellung, Eingabe und Windows-Laufzeit wurden hier nicht ausgeführt.

- Ein realer lokaler Spieler erhält nach der Laufzeit-Bereitschaft ein pausiertes
  Hauptmenü. Vorher bleiben Spielereingaben gesperrt, die Welt aber unpausiert,
  damit die Ankunftsprüfung weiterlaufen kann. Headless-Tests öffnen kein Menü.
- „Neues Spiel“ startet im unveränderten Startgebiet unmittelbar. Aus einer
  laufenden Sitzung verlangt es eine Bestätigung. Der Zielpfad kommt aus der
  tatsächlich konfigurierten GameDefaultMap und muss ein vorhandenes UWorld
  unter /Game sein; Entry, fehlende oder beliebige Assets reichen nicht aus.
- Beim Neustart bleiben Quell-Snapshot und bisherige Loot-/Händlerbestände bis
  zur bestätigten Bereitschaft einer anderen Zielwelt erhalten. Erst dann werden
  diese Sitzungsbestände zurückgesetzt. Ein fehlgeschlagener Weltwechsel nimmt
  entweder die unveränderte Quellwelt wieder auf oder übergibt den starken
  Snapshot an die bestehende Reise-Wiederherstellung. Spielstanddateien werden
  dabei weder gelöscht noch überschrieben. Erreicht die Zielwelt innerhalb von
  30 Echtzeitsekunden keine bestätigte Bereitschaft, übergibt ein unabhängiger
  Core-Ticker den aufbewahrten Stand ebenfalls an diese Wiederherstellung; eine
  fehlende Spielfigur kann daher keine unbegrenzte Neustart-Sperre verursachen.
  Fehlt außerdem ein lokaler Controller mit HUD, versucht die Übergabe einmal
  automatisch, die gesicherte Quellwelt zu öffnen. Es gibt keine automatische
  Wiederholungsschleife. Ist auch die Quellwelt nicht ladbar oder erzeugt keinen
  bedienbaren Controller, bleibt der Snapshot im GameInstance-Reisesystem
  aufbewahrt; dieser weitere Engine-/Inhaltsfehler kann ohne funktionierende
  Welt und Oberfläche nicht vollständig innerhalb des Spiels behoben werden.
- „Gespeicherten Spielstand laden“ verwendet die geprüfte jüngste Generation
  aus den beiden Speicherbänken und dem Legacy-Slot. Ein anstehender Reise- oder
  Lade-Snapshot hat immer Vorrang vor der Hauptmenü-Bootstrap-Sequenz.
- Pause → Hauptmenü erhält die laufende Sitzung; „Spiel fortsetzen“ kehrt zurück.
- Einstellungen umfassen vom System gemeldete Auflösungen, Fenstermodus,
  Grafikqualität, V-Sync, FPS-Limit, Gesamtlautstärke, Kameraempfindlichkeit und
  vertikale Invertierung. Nicht gespeicherte Änderungen lassen sich abbrechen.
  Anzeigeänderungen verlangen innerhalb von 15 Echtzeitsekunden „Beibehalten“;
  anderenfalls wird der vorherige Stand wiederhergestellt, auch bei Pause.
- Alle Menü-Buttons tragen die aktuelle Kontextgeneration. Ein Neuaufbau oder
  eine Wiederherstellung macht vorherige Aktionen ungültig. Beim Entfernen des
  Widgets werden ausstehende Einstellungen zurückgesetzt.

Portable Prüfung: `Tests/cpp/test_frontend_rules_v81.cpp` verwendet den tatsächlich
von der Implementierung eingebundenen `AskirFrontendRules.h` und prüft alle
Kombinationen der Start-, Bestätigungs-, Zielwelt- und Quellwelt-Voraussetzungen.
Die nativen Tests `Askir.Frontend.*` sind registriert, ohne Unreal aber nicht
als bestanden zu werten. Der zusätzliche Startmarker beweist nur, dass die
Laufzeitprüfung vor dem Menüaufbau erreicht wurde; er ersetzt keine Sicht- oder
Bedienprüfung.
