# Askir – Animation Blueprint und Charaktersteuerung V5

## Zehn umgesetzte Schritte
1. Native `UAskirAnimInstance` als Animation-Blueprint-Basisklasse erstellt.
2. Bewegungsgeschwindigkeit aus der vorhandenen AnimationState-Komponente übernommen.
3. Bewegungsrichtung für 2D-Blendspace übernommen.
4. Springen/Fallen als Animationsvariable verfügbar gemacht.
5. Sprintstatus als Animationsvariable verfügbar gemacht.
6. Leichte und schwere Angriffe an konfigurierbare AnimMontages angebunden.
7. Ausweichen an eine AnimMontage angebunden.
8. Blocken und Block-Ende an Montage und Status angebunden.
9. Trefferreaktion und Todesanimation an eigene Montages angebunden.
10. Wiederholte Aktionsauslösung per Sequenznummer sowie Ablaufzeit für kurzzeitige Aktionen ergänzt.

## Integration in Unreal Engine 5.6
1. Rigged-FBX mit Blender-Pipeline V4 erzeugen und in Unreal importieren.
2. Für das importierte Skeleton einen Animation Blueprint erstellen.
3. Bei **Parent Class** `AskirAnimInstance` auswählen.
4. Im AnimGraph einen Idle/Walk/Run Blendspace (Speed) mit einem `DefaultSlot`-Node verbinden.
5. Die Variablen `Speed`, `Direction`, `bFalling`, `bSprinting` für den AnimGraph verwenden.
6. Die neun importierten Animationsclips als AnimMontages vorbereiten (Block kann als Montage verwendet werden).
7. Im Animation-Blueprint die sechs Montage-Felder `LightAttackMontage`, `HeavyAttackMontage`, `DodgeMontage`, `BlockMontage`, `HitReactMontage`, `DeathMontage` zuweisen.
8. Im Spielercharakter den importierten Skeletal Mesh auswählen und `Anim Class` auf den neuen Blueprint setzen.
9. `DefaultSlot` muss mit dem Slot der Montages übereinstimmen. Nach Bedarf Animation Notifies und Root Motion ergänzen.
10. In PIE Laufen, Springen, Sprinten, Attacke, Blocken, Ausweichen, Schaden und Tod prüfen.

## Grenzen
- Keine echten Animation-Blueprint-`.uasset`-Dateien generiert; die nativen C++-Bindungen sind vorhanden, der AnimGraph muss im Unreal Editor angelegt werden.
- Die Blender-FBX-Exportpipeline V4 wurde hier nicht ausgeführt oder getestet.
- AnimMontages sind noch nicht vorhanden und werden in Unreal angelegt.
- Animation und Kampfschaden sind noch nicht framegenau synchronisiert; dafür sind Animation Notifies und ein Schadenfenster erforderlich.
- Automatisches Zurücksetzen der Aktionszustände erfolgt als Zeit-Fallback, nicht montage-synchron.
- Unreal-5.6-Kompilierung und Laufzeittest stehen aus.
