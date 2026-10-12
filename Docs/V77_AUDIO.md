# V77 – Originäre Audioquellen

Sieben WAV-Dateien wurden mit `Tools/generate_audio_v77.py` tatsächlich erzeugt und geprüft. Es handelt sich um synthetisierte ASKIR-Quellen, ohne Fremdaufnahmen, Samples oder übernommene Kompositionen. Die Quellen sind unter CC0-1.0 dokumentiert; Herkunft, Generatorhash und Dateihashes stehen in `ArtSource/Audio_V77/audio_manifest.json`.

| Quelle | Verwendung | Dauer |
| --- | --- | ---: |
| `S_Shard.wav` | Scherbe eingesammelt | 0,85 s |
| `S_Chest.wav` | Kellertruhe geöffnet | 1,25 s |
| `S_QuestAccepted.wav` | Maras Auftrag angenommen | 1,30 s |
| `S_QuestCompleted.wav` | Auftrag abgeschlossen | 2,05 s |
| `S_MeleeSwing.wav` | Waffenbewegung | 0,40 s |
| `S_MeleeHit.wav` | Nahkampftreffer | 0,55 s |
| `S_DungeonAmbient.wav` | Schleife für Keller/Dungeon | 8,00 s |

Alle Quellen sind unkomprimiertes 16-Bit-Mono-PCM bei 48 kHz. Tatsächlich geprüft wurden WAV-Header, vollständige Framezahl, Dauer, Nichtstille und Aussteuerung. RMS liegt zwischen 0,0518 und 0,1945, Spitzenpegel beträgt bei den Effekten 0,8200 und bei der Atmosphäre 0,3400. Die Effekte beginnen und enden mit Null; die periodisch komponierte Atmosphäre hat einen geprüften Grenzschritt von 0,00143. Alle sieben Dateien wurden separat erneut erzeugt und sind byteidentisch. Gesamtgröße: 1.382.708 Bytes.

Native Laufzeithooks spielen Scherben-/Truhenfeedback ausschließlich nach einem erfolgreichen Transfer und Maras Motive ausschließlich nach erfolgreicher Annahme bzw. Abgabe. Der zentrale `UAskirAudioLibrary` löst `/Game/Askir/V77/Audio/S_<Cue>` auf. Der V77-Kartengenerator importiert die WAV-Dateien als SoundWaves und platziert die Dungeonatmosphäre.

**Unreal-Import, Wiedergabe im Spiel, Mischung und manuelles Hörtesten in Unreal sind nicht ausgeführt**, weil Unreal Engine 5.6 fehlt. Es werden keine bereits existierenden SoundWave-Assets oder getesteten Laufzeitklänge behauptet. Die Quellen bilden eine kleine funktionale Dorf-Audiobasis; die umfangreiche Musik-, Wetter- und Umgebungswelt der Gesamtkampagne ist noch offen.

Erzeugung: `python Tools/generate_audio_v77.py`.
Reine erneute Dateiprüfung: `python Tools/generate_audio_v77.py --validate-only`.
