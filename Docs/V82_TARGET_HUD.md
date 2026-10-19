# V82: Zielerfassungsanzeige

Die native UMG-Anzeige in `AskirHUDWidget` liest das aktuelle Ziel direkt aus
`UAskirTargetLockComponent`. Es entstehen keine Widget-Blueprints, kopierten
Kampfwerte oder zusätzlichen dauerhaften Asset-Abhängigkeiten.

- Die Zielkarte zeigt den NPC-Namen sowie aktuelle und maximale Lebenspunkte
  aus dessen `UAskirCombatComponent`. Ein leerer NPC-Name erscheint als „Feind“.
  Der Lebensbalken wird auf 0–100 % begrenzt. Nicht endliche, nicht positive
  maximale oder bereits aufgebrauchte Lebenspunkte blenden die Anzeige aus.
- Die Karte sitzt oben mittig. Bei geringerer Breite wird sie gegenüber der
  bestehenden Spielerstatuskarte nach rechts beziehungsweise darunter versetzt.
- Eine kleine goldene Markierung folgt dem oberen Körperbereich des Ziels.
  Die Projektion nutzt `ProjectWorldLocationToWidgetPosition` und damit die
  DPI-Skalierung der UMG-Ansicht. Bei fehlgeschlagener Projektion, ungültigen
  Koordinaten oder außerhalb der sichtbaren Fläche bleibt die Markierung aus.
  Am Bildschirmrand werden keine irreführenden Ersatzmarkierungen erzeugt.
- Die Anzeige wird in jedem HUD-Tick aus dem aktuellen Ziel neu bestimmt. Ein
  zerstörter oder toter Gegner, ein verlorenes Ziel, Spielertod, geöffnete Menüs
  und ein ausstehender Weltwechsel blenden Karte und Markierung aus. Die Widgets
  empfangen keine Mausklicks.
- Der Steuerungshinweis nennt die mittlere Maustaste zur Zielerfassung und `R`
  zum Zielwechsel. Eingabe- und Kampfregeln liegen weiterhin beim Charakter und
  der Zielerfassungskomponente.

## Prüfstatus

Der Quellcode wurde auf vorhandene UMG-Abhängigkeiten, Widget-Lebensdauer,
ungültige Zielzeiger sowie endliche Gesundheits- und Projektionswerte geprüft.
Eine native Kompilierung, die Darstellung und tatsächliche Bildschirmprojektion
sind in der aktuellen Umgebung **blockiert**, weil Unreal Engine 5.6 fehlt.
Diese Anzeige gilt deshalb noch nicht als im Spiel getestet.

Folgende Prüfungen müssen im Unreal-Editor beziehungsweise Windows-Build
ausgeführt werden:

1. Einen aktiven feindlichen NPC erfassen: Name, LP und Markierung müssen zu
   genau diesem NPC gehören. Ein Zielwechsel muss alle drei Werte umstellen.
2. Schaden verursachen und den Gegner töten: LP müssen sinken; nach Tod darf
   keine Zielkarte und keine alte Markierung sichtbar bleiben.
3. Kamera drehen, Sichtgrenze und Reichweite überschreiten: Markierungen hinter
   der Kamera oder außerhalb des Bildes dürfen nicht am Rand hängen bleiben.
4. Pause, Inventar, Journal, Dialog, Handel, Magie und Einstellungen öffnen:
   Beide Zielelemente müssen verschwinden. Ein Weltwechsel und Spielertod
   müssen dasselbe bewirken.
5. 1920×1080, 1280×720 sowie unterschiedliche Windows-/UMG-DPI-Skalierungen
   prüfen. Karte und Körpermarkierung müssen an der vorgesehenen Stelle liegen.
6. Ein Ziel während seiner Erfassung zerstören und einen Spielstand laden:
   Es darf kein Zugriff auf einen alten Actor und keine übernommene LP-Anzeige
   erfolgen.
