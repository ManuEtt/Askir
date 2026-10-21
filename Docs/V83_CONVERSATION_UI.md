# V83: Native Dialogoberfläche

Die neue Gesprächsansicht in `UAskirHUDWidget` zeigt die Laufzeitdaten von
`UAskirConversationComponent`. `AAskirPlayerController::OpenConversation` kann
einen Sprecher und optional eine Dialog-ID öffnen. Ein neues Widget-Blueprint
oder ein zusätzliches dauerhaftes UI-Asset ist dafür nicht erforderlich.

## Darstellung und Auswahl

- Sprechername und Text stammen aus `GetCurrentNode()`. Angezeigt werden die
  von `GetAvailableChoices()` ermittelten Antworten. Das Backend entscheidet
  über ausgeblendete Bedingungen und den aktuellen Aktivierungszustand
  `bEnabled`; die Oberfläche berechnet keine eigenen Questbedingungen.
- Jede Schaltfläche hält ihre stabile `ChoiceID`, die `int64`-Revision der
  Unterhaltung und die Generation der aktuellen Menüansicht. Ein Klick gilt
  ausschließlich für diese Ansicht und ruft
  `SelectChoice(ChoiceID, ExpectedRevision)` auf. Listenpositionen lösen keine
  Dialogaktionen aus.
- Eine geänderte Backend-Revision erzeugt die Gesprächsansicht neu. Eine
  abgelehnte Auswahl zeigt den Fehlertext des Backends und aktualisiert die
  weiterhin gültige Unterhaltung. Knoten ohne weitere Antworten bieten nur
  „Gespräch beenden“ an.
- Die Ansicht blockiert Bewegung und Kampf wie die anderen Laufzeitmenüs,
  während die Welt weiterläuft. Die vorhandenen Hauptmenüs, Einstellungen,
  Zielanzeige und Maras gesonderte Brunnen-Questansicht bleiben erhalten.

## Integration von Torben

`ShowDeliveryDialogue` bleibt als bestehender Einstiegspunkt erhalten und
öffnet den Backend-Dialog `D_SQ_001`. Der bisher fest im HUD programmierte
Lieferungsdialog und seine direkten Annahme-/Abschlussaktionen wurden entfernt.
Dadurch laufen Torbens Entscheidungen und Questaktionen ausschließlich über
die geprüften Backend-Auswahlen. Fortschrittsanzeige und Questjournal lesen
weiterhin den wirklichen Zustand von `DeliveryMission`.

## Lebensdauer

Der HUD-Tick prüft `IsActive()` und `CanContinueConversation()`. Verliert das
Gespräch seinen gültigen Kontext, schließt sich das Menü. `ClosePanel` und
`NativeDestruct` beenden die Sitzung ausdrücklich; `CloseAnyMenu` erledigt das
auch beim Controller ohne erzeugtes HUD. So bleiben keine auswählbaren alten
Gesprächsschaltflächen nach einem Menüwechsel zurück.

## Prüfstatus und ausstehende Unreal-Prüfungen

Die Codeprüfung umfasst die Backend-API, stabile Auswahl-IDs, die Erneuerung
des Menükontexts und das Entfernen des alten direkten Torben-Aktionspfads.
`git diff --check` prüft die geänderten Dateien. Unreal-Kompilierung, tatsächliche
Widget-Darstellung und Benutzereingaben sind mangels Unreal Engine 5.6 in dieser
Umgebung **blockiert** und werden nicht als bestanden gewertet.

Im Editor und später im Windows-Build sind insbesondere zu prüfen:

1. Torben ansprechen, unterschiedliche Antworten wählen, ablehnen und erneut
   öffnen; Text und verfügbare Antworten müssen zum aktuellen Queststand passen.
2. Lieferung annehmen und abschließen; Gold, XP, Gegenstände, Entscheidungen
   und Fraktionswerte dürfen jeweils nur einmal durch die Backend-Aktion
   geändert werden.
3. Eine alte Antwort nach Knotenwechsel beziehungsweise Schließen und erneutem
   Öffnen auslösen; sie darf keine neue Auswahl oder Belohnung verursachen.
4. Den Gesprächspartner zerstören oder außer Reichweite bewegen, den Spieler
   töten sowie Speichern/Laden und Weltwechsel ausführen; ein ungültiges
   Gespräch muss geschlossen bleiben.
5. Den Dialog von Rurik sowie Knoten mit deaktivierten Antworten und ohne
   Folgeantworten prüfen. Danach Bewegung, Zielerfassung, Inventar, Hauptmenü
   und Einstellungen weiterhin regulär bedienen.
