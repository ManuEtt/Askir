# V83 – Laufzeitgespräche

Das neue `UAskirConversationComponent` verbindet validierte Dialoggraphen mit dem vorhandenen Spieler-, Quest- und Speichersystem. Die sichtbaren Texte und Verzweigungen stammen aus dem kompilierten Gesprächskatalog beziehungsweise aus vorhandenen `UAskirDialogueComponent::Nodes` eines Sprechers. Die laufende Sitzung bleibt vorübergehend; dauerhafte Erinnerungen liegen ausschließlich in den bereits gespeicherten Spieler-Dialogflags.

## Technisches Verhalten

Ein Gespräch benötigt einen lebenden, spielergesteuerten Charakter und einen gültigen Sprecher derselben Welt in Interaktionsreichweite mit freier Sichtlinie. NPCs müssen freundlich und aktiv sein; generische Sprecher benötigen eine Interaktionskomponente. Native Lieferungsaktionen sind ausschließlich am tatsächlichen `AAskirDeliveryQuestActor` mit Rolle `Giver` erlaubt. Verdeckte, zerstörte oder deaktivierte Sprecher, Weltabbau, Pause, Reisen und laufende Speicheroperationen sperren Auswahl und Beginn.

Graphen werden vor Installation vollständig geprüft: höchstens 64 Knoten, 32 Antworten pro Knoten, 32 Einstiegspunkte und jeweils 16 erforderliche oder ausgeschlossene Flags. Knoten- und Antwort-IDs müssen im jeweiligen Gültigkeitsbereich eindeutig sein; sämtliche Ziele und Einstiegspunkte müssen existieren. Erforderliche und ausgeschlossene Flags dürfen sich nicht widersprechen. Einstiegspunkte werden in ihrer verfassten Reihenfolge gegen Lieferungsphase und gespeicherte Flags ausgewertet.

Eine Antwort trägt eine stabile ID und einen monotonen 64-Bit-Sitzungsstand. Jede erfolgreiche Wahl beziehungsweise jeder Sitzungswechsel verwirft ältere Buttons. Überlauf wird abgelehnt. Vor dem Aufruf externer Quest-, Fraktions- oder Belohnungsdelegates werden neuer Sitzungsstand und Transaktionsschutz installiert. Währenddessen kann keine weitere Auswahl, neue Sitzung oder unvollständige Speicherung durchgeführt werden. Ein Schließen während eines Callbacks wird vorgemerkt und anschließend ausgeführt. Zerstört ein Callback Sprecher oder Spieler, bleibt eine bereits verbuchte Questaktion erhalten und die Sitzung wird sicher geschlossen.

`accept_delivery` ruft die vorhandene, abgesicherte Mission mit `help` oder `bargain` auf. `return_delivery` verwendet deren bestehenden Abschluss und Belohnungsbeleg. `quest_start` ist ausschließlich für `SQ_001` implementiert und entspricht der Hilfsentscheidung. Andere deklarierte Questaktionen bleiben gesperrt. Es werden keine fehlenden Quests durch Dialogtexte als spielbar ausgegeben und keine neuen Belohnungspfade eingeführt. Graphen dürfen keine `SQ_`-Flags vergeben; kanonische Missionsbelege stammen weiterhin aus dem Missionssystem.

Antwortbedingungen lesen das Spielerflagbuch. Ein neues Erinnerungsflag wird erst nach erfolgreicher Aktion eingetragen. Bei fehlschlagender Aktion bleiben aktueller Knoten und bestehende Flags erhalten. Eine überfüllte Tasche sperrt die Lieferungsabgabe bereits in der UI und weiterhin an der Backend-Grenze.

Bestehende Blueprint-Dialogknoten werden bei Bedarf übernommen. Ihre Antwort-IDs entstehen deterministisch aus dem Index innerhalb des Knotens. Ein konfiguriertes `AAskirNPC::ConversationID` hat Vorrang vor diesem Rückfallpfad. `IsKnownDialogue` ermöglicht dem Editor die Prüfung des tatsächlich kompilierten Katalogs.

## Prüfung und Grenzen

`Tests/cpp/test_conversation_rules_v83.cpp` führt produktive Graph-, Flag-, Aktions-, Sitzungsstand- und Callback-Abschlussregeln unter dem lokalen C++-Compiler aus. Die Prüfungen umfassen beschädigte und übergroße Graphen, sämtliche Kombinationen von vier erforderlichen/ausgeschlossenen/vorhandenen Flags, alle 24 deklarierten Nebenquestaktionen, veraltete und rekursive Auswahlbedingungen, 10.000 fortlaufende Sitzungsstände und die Überlaufgrenze. Das genaue Ergebnis steht im V83-Verifikationsbericht.

Fünf native Unreal-Tests sind zusätzlich registriert: Legacy-Konvertierung und Spielerflags, Torbens tatsächlicher Questablauf inklusive serieller Speicherung, Sprecher-/Sichtlinien-/Aktionsrechte, volle Tasche und erneuter Abgabeversuch sowie reale Questcallbacks mit Rekursion, Speicher-/Ladeversuch, verzögertem Schließen und Actor-Zerstörung. Diese Tests benötigen Unreal Engine 5.6 und wurden in der aktuellen Umgebung **nicht ausgeführt / blockiert**.

Diese Änderung liefert implementierten Laufzeitcode und ausführbare portable Nachweise. Unreal-Kompilierung, UMG-Bedienung im Editor, generierte Weltassets und ein gestarteter Windows-Build benötigen weiterhin die Engine- und Windows-Abnahme.
