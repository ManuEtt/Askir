# Askir RPG V33

Basis: V32. Neuer Schwerpunkt: Kill-Attribution und Quest-HUD-Events.

- `UAskirCombatComponent::GetLastDamageCauser()` speichert die letzte positive Schadensquelle.
- `AAskirDornfeldMissionRelay` rechnet Wächter nur dem konfigurierten Spieler bzw. dessen Actor/Projektil zu.
- `UAskirDornfeldMissionComponent::OnObjectiveChanged` liefert Text und Stage für ein UMG-HUD.
- Neue QA-Dokumentation und ergänzte statische Missionstests.

Die C++-Quellen sind nicht in Unreal kompiliert und das HUD-UAsset ist nicht automatisch erzeugt.
