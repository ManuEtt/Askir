# GitHub-Übertragung V80

Am 19.10.2026 wurde der V80-Stand erfolgreich zu `ManuEtt/Askir` übertragen.

- Lokaler Entwicklungscommit: `0ce4f78f820dfaf9666e5a34f9cce29e9c457f37`.
- GitHub-Commit auf dem aktuellen Repository-Verlauf: `30b118fd9b59aeaa5f3fccfd74144872c13ea48f`.
- Ausgangspunkt auf GitHub: `7f57122e76f817fe113c193e0ff93a887fbfc37a`.
- Branch: https://github.com/ManuEtt/Askir/tree/development/v80-outskirts-delivery
- Draft Pull Request: https://github.com/ManuEtt/Askir/pull/1
- Erfolgreicher GitHub-Actions-Lauf: https://github.com/ManuEtt/Askir/actions/runs/39196378507

Die Änderungen von V79 und V80 wurden als Delta auf den tatsächlichen GitHub-main angewendet. Die bereits entfernten veralteten Root-Skripte wurden nicht wiederhergestellt. Die 39 bestehenden FBX-LFS-Zeiger blieben unverändert; ihre lokalen Rohdaten wurden vor der Upload-Prüfung gegen GitHub-OID und Größe materialisiert.

Im Upload-Checkout bestanden 555 Python-Tests und 155.246 portable C++-Assertions. GitHub Actions bestätigte ebenfalls den kompletten Offline-Prüfer einschließlich LFS-Checkout. Das belegt weder Unreal-Kompilierung noch Spielbarkeit: UE 5.6, echte Karten-/Assetproduktion und Win64-Abnahme bleiben offen.

Der Pull Request ist als Entwicklungsstand markiert. `main` wurde nicht ohne Review überschrieben oder erzwungen aktualisiert. Frühere HTTP-403-Protokolle dokumentieren frühere Verbindungsrechte und sind durch diesen erfolgreichen Upload überholt.
