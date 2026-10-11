# Askir RPG V59 – Unterbrechbare Bossangriffe

Aufbauend auf V58: Die Boss-KI bindet sich an das Damage-Event des eigenen CombatComponent. Ein positiver Treffer während einer laufenden Angriffsvorwarnung kann den Angriff ab einer einstellbaren Schadensschwelle abbrechen und eine Stagger-Pause auslösen. Blueprint-Event `OnBossAttackInterrupted` für Hit-Reaction/VFX; noch keine fertigen Animation Assets. Bei Controllerwechsel wird die Delegate-Bindung entfernt. Während Stagger darf kein neuer Angriff starten. Die vorhandene Trefferauflösung und Sichtlinienprüfung bleiben erhalten.

**Grenzen:** Nur während des Windups unterbrechbar, nicht nach bereits gestartetem Attack-Impact. Die 5 JSON-Profile sind Konfiguration, nicht automatisch auf Blueprint-CDOs angewendet. Keine Unreal-Kompilierung oder PIE-Ausführung. Die 50 Testszenarien sind Daten-/Vertragsprüfungen und kein Unreal-Laufzeittest.
