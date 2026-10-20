# V82 – Verifikation und offene Engine-Abnahme

Der tatsächliche Offline-Lauf wird in `Reports/V82_verification.json` dokumentiert.
Seine Quelldateihashes müssen mit dem Paket übereinstimmen. Der echte
Umgebungsaufruf des V82-Buildrunners liegt unter
`Reports/V82/pipeline_environment.json`: UE 5.6 fehlt, der Host ist Linux.

| Prüfung | Status |
| --- | --- |
| Python-Regressionen | Bestanden: 587 Tests |
| Portable C++-Produktionsregeln mit g++/UBSan | Bestanden: 414.886 Assertions in zwölf Programmen |
| Originale Rig-/Animations-/Prop-/Audioquellen | Bestanden |
| Unreal Header Tool und UE-C++ | Blockiert: Engine fehlt |
| Native Askir-Automation | 61 Tests registriert, nicht ausgeführt |
| HUD-Darstellung, Eingaben und Kamera | Nicht ausgeführt |
| Assetimport und echte `.umap`-/`.uasset`-Erzeugung | Blockiert: Engine fehlt |
| Win64-Paket und externer Spielstart | Blockiert: Engine und Windows-Host fehlen |
| Durchspielen von Dorfmission und Nebenquest | Nicht ausgeführt |
| Performance / 60 FPS | Blockiert; keine FPS-Messung |

Die neue Startprüfung verlangt Weltbereitschaft, Titelmenü und konfigurierte,
registrierte TargetLock-Komponente für denselben Spieler. Sie lehnt alte,
unvollständige oder widersprüchliche Marker ab. Ein solcher Startnachweis prüft
weder Mausbedienung noch Kampftreffer oder Kameragefühl.

## Manuell nach einem erfolgreichen Windows-/Unreal-Build

1. Im Hauptmenü neues Spiel beginnen. Zivilisten und inaktive Wächter dürfen
   nicht als Kampfziel erfasst werden; mittlere Maustaste ohne Ziel bleibt nutzbar.
2. In der Mara-Mission alle drei Scherben sammeln. Einen aktiven Wächter erfassen,
   seitlich laufen, Kamera und Figur prüfen. Name, LP und Markierung müssen zum
   tatsächlichen Gegner gehören; mit R wechseln und per mittlerer Taste lösen.
3. Leichte/schwere Angriffe und Projektile gegen aktive Gegner einsetzen.
   Ausweichen, Schild, inaktive Ziele und Wände dürfen kein falsches Hitfeedback
   erzeugen. Zivilisten dürfen durch gewöhnliche Schläge keinen Schaden nehmen.
4. Vor eine Mauer laufen: verdeckte Ziele nicht neu erfassen; bestehende Auswahl
   nach kurzer Sichtfrist lösen. Weder Nahkampf noch Projektil darf durch die
   Mauer treffen. Auch eine Wand direkt vor dem Wirkungsursprung muss blockieren.
5. Front-, Seiten- und Rückentreffer bei aktivem Block prüfen. Fehlende native
   Ausdauer darf nicht auf den zweiten Legacy-Ausdauerpool ausweichen. Geschosse
   müssen nach Flugrichtung beurteilt werden, auch wenn ihr Schütze sich bewegt.
6. Menü öffnen, speichern/laden, sterben und Dorf/Umland wechseln. Auswahl muss
   gelöscht sein und die ursprünglichen Bewegungsflags müssen wieder gelten.
7. Die gesamte Brunnenmission abschließen und 90 Gold / 180 XP prüfen; speichern,
   Prozess neu starten und erneut laden. Danach Torbens Nebenquest durchspielen.
8. HUD bei 1280×720 und 1920×1080 mit verschiedenen UI-Skalierungen prüfen.
   Zielanzeige, bestehender Statusblock und Menü dürfen sich nicht verdecken;
   hinter der Kamera liegende Ziele dürfen keinen alten Marker zurücklassen.
9. Auf dokumentierter Windows-Hardware Bildrate, Speicher und Ladezeit messen.

Jeder Punkt bleibt bis zu einem protokollierten Engine-Lauf **Nicht ausgeführt**.
Die ZIP ist ein Entwicklungsprojekt und kein fertig abgenommenes Windows-Spiel.
