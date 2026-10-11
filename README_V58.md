# Askir RPG V58 – Boss-Angriffsvorwarnung

Die Boss-KI plant nun einen Angriff erst nach Ablauf der Vorwarnzeit. Die vorhandene Trefferauflösung in `AAskirCharacter` bleibt erhalten. Nach Ablauf der Vorwarnung werden Ziel, Reichweite und Sichtlinie erneut geprüft. Bei Tod, Controllerwechsel oder explizitem Reset wird der Timer abgebrochen.

**Grenzen:** Die Angriffsvorwarnung ist ein optionales Blueprint-Event; Animationen und Effekte sind nicht enthalten. Die tatsächliche Schadensauflösung erfolgt weiterhin über `AAskirCharacter::ResolvePendingAttackImpact` und dessen Timer/AnimNotify. Unreal Engine 5.6 wurde nicht ausgeführt.

Editor: C++ kompilieren, Boss-Controller zuweisen, `OnBossAttackTelegraph` im Controller-Blueprint implementieren, Testkampf im PIE ausführen.
