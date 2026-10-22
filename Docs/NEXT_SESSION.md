# Fortsetzung nach dem V84-Zwischenstand

Auf Wunsch des Nutzers endet dieser Arbeitsabschnitt nach Sicherung des geprüften
V84-Stands. Keine weitere Ausbaustufe beginnen, bevor der gesicherte Stand und
seine offenen Engine-Abnahmen gelesen wurden.

- Entwicklungsbranch: `development/v84-herbs`. Exakter Commit und Dateihashes
  stehen im ZIP-`DELIVERY_MANIFEST.json`; Liefernachweis liegt neben dem Archiv.
- Zuerst `git status`, `Docs/V84_CHANGES.md`, `Docs/V84_VERIFICATION.md` und den
  aktuellen `Reports/V84_verification.json` lesen. Bereits implementierte
  Änderungen nicht erneut erzeugen. Vor GitHub-Arbeit aktuellen main und PR-Status
  abrufen, Benutzeränderungen erhalten und keine bereits gemergten PRs umschreiben.
- Leitprüfung: `python Tools/verify_v84.py`. Vor erneutem Verpacken müssen alle
  Offline-Prüfungen bestehen und `verified_inputs` mit den aktuellen Quellen
  übereinstimmen. Native Tests sind hier erstellt, aber nicht ausgeführt.
- UE 5.6/Windows fehlen. Sobald verfügbar, `Scripts/Build-AskirV84.ps1` auf einem
  Windows-Host ausführen und die tatsächliche manuelle Abnahme dokumentieren.
  Keine `.umap`-/`.uasset`-/Spielbarkeitsnachweise aus JSON oder Quellcode ableiten.
- SQ_003: Rurik mit unveränderter `ConversationID=D_DORNFELD_RURIK` und neuem Tag
  `ASKIR_V84_RURIK_GIVER`; Quelle `SQ003_Meadow_V84`; Originalreward 24 Gold,
  150 XP, einmal `item_004`; Format 5. Virtuelle `SQ_003_STAGE_*`-Bedingungen sind
  nicht maßgeblich gespeichert. Der Questledger bleibt die einzige Zustandsquelle.
- Karten bleiben `Dornfeld_V80` und `Dornfelder_Umland_V80`. Die Editor-Nachschritte
  ergänzen bestehende Weltverträge und bewahren Actors. Es gibt hier weiterhin
  keine engine-erzeugten Karten. Vollständige Kampagne und 30 Level bleiben offen.
- Originale Kräuterquellen stehen in `ArtSource/Environment_V84`; Herkunft/CC0
  und echte Binärprüfungen sind dokumentiert. Import erst durch Unreal belegt.
- Danach den nächsten zusammenhängenden Spielabschnitt wählen; bestehende
  kanonische Daten und tatsächlich behobene Integrationen haben Vorrang vor
  zusätzlicher Menge ungetesteter Daten. Keine Käufe ohne Zustimmung.
