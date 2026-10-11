# Askir V64 – Wiederverwendung und Build-Vorbereitung

## Implementiert
- Boss-Controller stellt beim erneuten Possess die in der Blueprint-Klasse eingestellten Angriffswerte wieder her; Phasenmultiplikatoren kumulieren nicht mehr über mehrere Bosse.
- Tod des Bosses stoppt offene Vorwarnungen und ausstehende Treffer; die Boss-Tick-Verarbeitung endet für diesen Frame.
- UnPossess bricht offene Treffer ab und setzt den AnimNotify-Modus zurück.
- Neues Offline-Audit: `python Tools/audit_build_readiness_v64.py` prüft Pflichtdateien, UE-5.6-Zuordnung, JSON und Python-Syntax.
- 50 dokumentierte PIE-Szenarien in `RELEASE_V64_50_SCHRITTE.md`.

## Noch nicht validiert
Unreal-C++-Kompilierung, Animation-Assets, Editor-Python-Skripte, PIE/Gameplay. Die automatisierten Offline-Tests ersetzen diese Schritte nicht. GitHub wird nicht verändert.
