# V78 – Teststatus des Zwischenstands

Die aktuelle maschinenlesbare Gesamtprüfung steht in `Reports/V78_verification.json`. Die Ergebnisse beziehen sich auf die tatsächlichen Quellen und Dateien des Zwischenstands.

| Prüfung | Status | Aussage |
|---|---|---|
| Quellstruktur, Reflexionsnamen, Paketbestand | Bestanden | Statische Strukturprüfung; kein UHT-/UE-Build |
| Python-Regressionen | Bestanden | Anzahl und Ausführung im aktuellen JSON-Bericht |
| Produktions-Missionsregeln | Bestanden | 73 echte C++-Assertions |
| Produktions-Rechenregeln | Bestanden | 511 echte C++-Assertions |
| Produktions-Magieregeln | Bestanden | 451 echte C++-Assertions, 24 kanonische Zauber |
| Neun WAV-Quellen | Bestanden | PCM, Dauer, Aussteuerung, Dateiintegrität |
| 39 FBX-Quelleingaben | Bestanden | Existenz, FBX-Header und SHA-256; kein UE-Import |
| Drei GLB-Skins und 36 Clips | Bestanden | Blender-Export/Import und unabhängiges CPU-Skinning der tatsächlichen Buffer |
| Native Unreal-Automation | Blockiert | Engine fehlt; registrierte Testnamen im Gesamtbericht |
| Echte Karten-/Assetgenerierung | Blockiert | Null `.umap` und `.uasset`; der Editor wurde nicht ausgeführt |
| Win64-Build und Start | Blockiert | UE5.6-/Windows-Werkzeuge fehlen |
| Gameplay, UI, Audio und FPS im Spiel | Nicht ausgeführt | Kein lauffähiger Unreal-Build |

Die Charakterbilder in `ArtSource/Animated_V78/Previews` sind Blender-Quellvorschauen. Sie sind keine Unreal- oder Windows-Spielbilder. Alle Blender- und CPU-Prüfungen sowie ihre Grenzen stehen in den separaten Characterberichten.

ZIP-Zwischenstände werden zusätzlich per CRC und SHA-256 jedes Dateieintrags gegen die Projektquelle geprüft. Enthaltene FBX-Dateien sind tatsächliche materialisierte Binärdaten; Git-LFS-Zeiger werden vom Packager abgelehnt. V77 und V78-Checkpoint01 bleiben als separate Sicherungen erhalten.
