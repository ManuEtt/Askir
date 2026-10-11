# Askir RPG – 10 Animations-Integrationsschritte (V3)

1. Eigenständige Animation-State-Komponente erstellt.
2. Bodenbewegungsgeschwindigkeit aus Character-Velocity abgeleitet.
3. Bewegungsrichtung in lokalen Gradwerten berechnet.
4. Sprung-/Fallstatus aus CharacterMovement ermittelt.
5. Sprintstatus mit konfigurierbarer Geschwindigkeitsschwelle ermittelt.
6. Leichte Angriffe als Action-State aus dem bestehenden Kampfsystem ausgelöst.
7. Schwere Angriffe als eigener Action-State ausgelöst.
8. Ausweichen und Blocken mit Animation-State verknüpft.
9. Trefferreaktion bei empfangenem Schaden signalisiert.
10. Tod als endgültigen Animationszustand erfasst.

## Unreal-Anbindung
Die Komponente `AnimationState` ist auf `AAskirCharacter` vorhanden. Ein AnimBlueprint kann `GroundSpeed`, `DirectionDegrees`, `bIsInAir`, `bIsSprinting`, `Action` und `ActionSequence` auslesen. `ActionSequence` wird bei jedem neuen Ereignis erhöht, damit auch gleiche aufeinanderfolgende Angriffe erkannt werden. Nach einem AnimMontage-Ende muss `FinishAction()` aufgerufen werden. Für den Tod bleibt der Zustand bestehen.

## Wichtige Grenzen
Die vier bisherigen Charaktermodelle sind **statische, nicht geriggte Vorschau-Meshes**. Es wurden **keine** Skelette, Animationen, AnimMontages oder Unreal-AnimBlueprint-Assets erzeugt. Die neue Komponente liefert Daten und Ereignisse zur späteren AnimBlueprint-Integration. Ohne echte Skeletal Meshes und Animationsassets wird noch keine Charakteranimation abgespielt. Der Code ist nicht in Unreal Engine 5.6 kompiliert oder getestet. Bei einem Import müssen Maße/Skalierung geprüft werden.
