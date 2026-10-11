# Askir V53 – Individuelle Gegner-KI

**Themen 1401–1450.** Sechs Gegner-Blueprints erhalten eigene AIController-Blueprint-Klassen und unabhängige Presets. Anders als in V52 teilen sie nicht mehr zwangsläufig dieselben AI-Einstellungen.

## Ablauf in Unreal Engine 5.6
1. Projekt sichern, C++ kompilieren, Python Editor Script Plugin aktivieren.
2. `Tools/integrate_blueprints_v51.py` nach Prüfung mit `DRY_RUN=False` ausführen.
3. `Tools/configure_individual_ai_v53.py` im Editor zunächst mit `DRY_RUN=True` ausführen.
4. Ausgabe prüfen, dann `DRY_RUN=False` setzen und erneut ausführen.
5. Bereits vorhandene individuelle Controller werden standardmäßig nicht verändert; nur nach explizitem `APPLY_EXISTING_PRESETS=True`.
6. NavMesh, Spawning, Sichtlinien, Kampf und Speichern in Play-In-Editor testen.

**Grenzen:** Kein Unreal Editor/Compiler hier vorhanden; Skript und C++ nicht in UE ausgeführt. Keine fertigen Behavior Trees, Animationen oder `.uasset`-Dateien. Sechs Presets sind vorläufige Designwerte.
