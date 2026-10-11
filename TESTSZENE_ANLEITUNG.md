# Askir – Das verlassene Lager (UE 5.6)

## Inhalt
Eine **zur Laufzeit in C++ aufgebaute Testarena** mit Boden, Begrenzungswänden, Hindernissen, einem Lagerwächter, einer Vorratskiste, einem Lagerfeuer, einem Ausgang und einem feindlichen Plünderer. Es ist keine zusätzliche `.umap` erforderlich. Die Startkarte bleibt `/Engine/Maps/Entry`.

## Steuerung
- WASD: Bewegen, Maus: Kamera, Leertaste: Springen, Shift: Sprinten
- Linksklick: Angriff, Rechtsklick: schwerer Angriff, Q: Blocken, Alt: Ausweichen
- E: Interagieren (maximal 260 Unreal-Einheiten vom Marker entfernt)
- F5: Schnellspeichern, F9: Schnellladen

## Spielablauf
1. Zum Lagerwächter gehen und E drücken.
2. Zur Vorratskiste gehen und E drücken; drei Rationen landen im Camp-System.
3. Den Plünderer besiegen. Bei Sieg werden 50 XP vergeben.
4. Am Lagerfeuer mit E rasten und Lebenspunkte/Ausdauer auffüllen.
5. Zum Ausgang gehen und E drücken; Abschluss bringt 100 XP.

## Unreal-Start
1. Unreal Engine 5.6 und Visual Studio 2022 mit C++-Workload installieren.
2. `AskirRPG.uproject` öffnen, Projektdateien erzeugen und C++ kompilieren.
3. Auf der Standard-Entry-Map **Play** starten. Die Arena wird im GameMode automatisch erstellt.
4. Falls keine Spielfigur erscheint: eine leere Map mit PlayerStart anlegen und GameMode `AskirGameMode` einstellen. Die Arena wird weiterhin erzeugt.

## Grenzen
- **Nicht kompiliert oder in Unreal getestet.** Dies ist eine ausführbare Testszene als C++-Implementierung, keine bestätigte spielbare Build-Datei.
- Kein ausgearbeitetes Level-Asset, keine finalen 3D-Modelle, Animationen, Dialogfenster oder Inventar-UI.
- NPC-KI benötigt je nach Navigation einen NavMeshBoundsVolume; ohne diesen kann der Gegner möglicherweise nicht navigieren. Der Nahkampf bleibt als Testziel verfügbar.
- Die Kiste speichert Demo-Rationen und füllt den Camp-Vorrat, nicht ein eigenes DataAsset im Inventar.
- Speichern/Laden deckt die Demo-Quest, Vorräte und besiegten Gegner ab; ein bereits besiegter Gegner kann im selben Lauf nicht wiederbelebt werden, wenn ein älterer Spielstand geladen wird.
- Vor der Integration in den Hauptzweig ist ein Unreal-C++-Build und ein vollständiger Testlauf erforderlich.
