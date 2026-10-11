# Askir RPG V60 — Boss-Animationsintegration

Kumulativer Stand V59 + V60. C++-Integration für optionale Angriffs- und Trefferreaktions-Montagen, Animation-Notify-Trefferfenster, Timer-Fallback bei fehlendem Montage-Start, Abbruch offener Angriffe bei Stagger. Fünf Profile enthalten **keine erfundenen Unreal-Assets**.

## Einrichtung in Unreal 5.6
1. Projekt mit Visual Studio / Unreal Build Tool kompilieren.
2. Für Bossgegner AnimInstance mit Montage Slot einrichten und AttackMontage / StaggerMontage am AIController konfigurieren.
3. Optional `Askir Attack Impact` AnimNotify exakt am gewünschten Trefferframe einfügen und **erst dann** `bImpactFromMontageNotify=true` setzen.
4. Ohne Montage bleibt die bestehende zeitgesteuerte Schadensauflösung aktiv.
5. Animationen, VFX, Sound und Boss-Blueprints sind nicht enthalten und müssen importiert bzw. konfiguriert werden.

## Prüfungen
`python Tools/validate_boss_animation_v60.py`
`python -m unittest discover -s Tests -p 'test_boss_animation_v60.py'`

Keine UE-Kompilierung oder Spieltests in dieser Umgebung. Bestehende V59-Funktionen unverändert übernommen, außer den dokumentierten C++-Erweiterungen.
