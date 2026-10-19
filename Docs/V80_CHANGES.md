# V80 · Dornfelder Umland und verlorene Lieferung

## Erstellte Implementierung

V80 setzt auf Commit `5a7f472` auf. Die V79-Auslieferung wurde vor Änderungen als `AskirRPG_V79_PreV80.zip` gesichert. Die ursprünglichen V77–V79-Quellen und Assetquellen bleiben erhalten.

- `UAskirDeliveryMissionComponent` bindet SQ_001 an die vorhandenen Questzustände, persistenten Ereignisbelege und Belohnungsquittungen. Der kanonische Platzhalterablauf erhält zwei konkrete Banditen-IDs und einen zusätzlichen Frachtsicherungsschritt. Kein zweiter, abweichender Quest-Speicher wurde eingeführt.
- Torben bietet einen bestätigten Dialog mit Information, Annahme, Ablehnung und Rückgabe. Helfen gibt einmalig Waldhüter +2, Verhandeln einmalig Freie Händler +2. Der kanonische Abschluss vergibt 100 XP, 16 Gold und `item_002` genau einmal.
- Kampfrelays prüfen tatsächliche Spielerzuordnung, bewahren einzigartige Todesbelege, deaktivieren bestätigte Gegner nach Kartenwechsel und stellen sie bei älteren Spielständen wieder her. Verspätete Bindung und unzugeordnete Todesfälle bleiben erneut lösbar. Die beiden Missionsbanditen vergeben keine separaten, ungesicherten Killbelohnungen.
- Native Dialog- und Rufkomponenten sind nun am Spieler vorhanden und werden durch die bestehende Save-Zuordnung tatsächlich erfasst. NPCs erben dieselbe Dialogkomponente, statt ein namensgleiches Subobject erneut anzulegen.
- Die Fracht ist ein echter Interaktionsaktor. Sie kann erst nach den beiden bestätigten Kämpfen gesichert werden; bloße Nähe schließt die Quest nicht ab. Der untergeordnete Visual-Offset trennt logische Interaktionsposition und Boden-Pivot des originalen Kistenmodells.
- Questbelohnungen bereiten alle XP-/Level-/Punkteänderungen vor. Überlauf, ungültige Werte und fehlender Inventarplatz scheitern ohne Teilzahlung oder vorzeitige Abschlussquittung. Fehlgeschlagene Lieferungsabgabe stellt auch den vorherigen Zielzustand wieder her.
- Speichern/Laden prüft aktive Quests, Zielspiegel, Wahlbelege, Frachtschlüssel und Killbelege vor Änderungen. Laufende Quest-/Lieferungs-Transaktionen und ausstehende Reisen verhindern konkurrierende Disk-Operationen. Vor Restore-Rückrufen werden Gegner und Fracht mit dem neuen Stand abgeglichen.
- Portale prüfen lebenden Spieler, tatsächliche Distanz, Zugangsbedingungen und existierende `UWorld`-Ziele. V80 verwendet explizite E-Interaktion, während ältere Karten ihre Überlappungsoption behalten. Ankunfts-ID und Gegenroute müssen eindeutig passen; eine fehlende explizite Ankunft wird nicht still auf einen anderen Spawn umgebogen.
- Das GameInstance-Snapshot bleibt bei gescheiterter Ankunft erhalten. Quelle und Ziel werden über echte Weltidentität getrennt; nur eine weiterhin gültige Quellwelt darf einen gescheiterten Start freigeben. Wiederholung und Rückkehr erhalten die gesicherten Ressourcen.
- Ausstehende Reisen frieren Eingaben und Bewegung einschließlich Physik ein, verhindern eingehenden Spielerschaden und pausieren feindliche Entscheidungen. Ein abgewiesener Reisestart stellt die vorherige Bewegungsart wieder her; ein erfolgreicher Restore stellt lebenden oder toten Zustand gezielt her.
- Neue UI zeigt Region, Lieferziel, beide Quests im Journal und die Fraktionsauswirkung. Aktionsbuttons einschließlich Pause, Dialog und Wiederherstellung tragen Kontextgenerationen. Die Reise-Recovery bleibt auch bei fehlendem Ziel-Pawn bedienbar.
- Die Editorpipeline erzeugt zwei zusammenhängende Blockout-Karten mit Bauernstraße, Lager, Torben, echten Relays, Fracht und Rückweg. Startup- und Cook-Konfiguration werden erst nach Validierung beider tatsächlicher Packages gesetzt. Die Originalmodelle werden verpflichtend importiert; ein fehlerhafter Import wird nicht durch einen Würfel versteckt.

## Tatsächlich erzeugte Assets

Unter `ArtSource/Delivery_V80` liegen zwei originale Modelle mit insgesamt 7.764 Dreiecken in Blender-, GLB- und OBJ-Form, MTL-Materialien und echten Render-Vorschauen. Lizenz, Herkunft und Hashes liegen dabei. Der Handwagen besitzt statische Räder; seine einfache Unreal-Boxkollision ist für eine Requisite vorgesehen. Es ist kein fahrbares Fahrzeug.

Die neuen Modellquellen verwenden keine heruntergeladenen Inhalte. Vorhandene Engine-Grundformen in der Weltpipeline unterliegen weiterhin der Unreal-Lizenz und sind keine neuen CC0-Assets. Torben, Landschaft und Gebäude bleiben Blockout-Darstellung; Bauernhäuser haben noch keine begehbaren Innenräume.

## Konkrete Blockaden und verbleibender Umfang

UE 5.6, UHT/UBT, Editor-Python und ein Windows-Host stehen hier nicht bereit. Deshalb wurden die beiden Karten nicht als `.umap` serialisiert, keine `.uasset`-Dateien importiert, keine Unreal-Klassen kompiliert und keine Spielsequenzen tatsächlich durchgespielt. Auch die registrierten Unreal-Tests stehen aus. Offline-Regeltests und Quellverträge ersetzen diese Nachweise nicht.

Die vollständige Kampagne, 30 spielbare Levels, mindestens 20 abgenommene Nebenquests, drei Enden, Produktionsgrafik/-animationen, vollständige Audio-/VFX-Integration und Windows-Paketierung sind weiterhin nicht erfüllt. FPS, Ladezeiten und Spielspeicherbedarf wurden mangels ausführbarem Spiel nicht gemessen. Es wurde kein Windows-Spielarchiv vorgetäuscht.

## Nächste Engine-Abnahme

Nach erfolgreicher V80-Pipeline beide Dorfmissionen mit realen Eingaben spielen. SQ_001 vor/nach jedem Kill, vor/nach Frachtaufnahme und vor/nach Rückgabe speichern. Zwischen beiden Karten hin und zurück reisen und denselben Spielstand mehrfach laden. Zusätzlich volle Taschen, ungültige Daten, NPC-/Umweltschaden, Ablehnung/erneute Annahme, fehlgeschlagenen Reisebeginn und blockierte Zielankunft prüfen. Den paketierten Build separat außerhalb des Editors durchspielen.
