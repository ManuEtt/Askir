# Askir RPG V50 – Blueprint-Basisbibliothek

Dieses Release ergänzt das kumulative Askir-Quellprojekt um **24 konkrete Blueprint-Asset-Definitionen** und ein Unreal-Editor-Erstellungsskript. Die Blueprint-Assets werden **nicht** im ZIP als `.uasset` mitgeliefert: Sie werden erst durch Ausführen des Skripts im Unreal Editor erzeugt.

## Vorbereitung
1. Askir in Unreal Engine 5.6 öffnen und C++-Projekt erfolgreich kompilieren.
2. Editor Scripting Utilities und Python Editor Script Plugin aktivieren, Editor neu starten.
3. `Tools/generate_blueprints_v50.py` im Unreal-Python-Editor ausführen. Standard: `DRY_RUN=True` (nur Vorschau).
4. Erst nach Sichtung der Vorschau `DRY_RUN=False` setzen und erneut ausführen.
5. Im Content Browser `/Game/Askir/Blueprints` prüfen; fehlende Elternklassen werden protokolliert.

## Umfang
- Core: Player, GameMode, NPC, erweiterter NPC
- AI: Enemy-/Civilian-AI-Controller
- Enemies: Bandit, Wolf, Skeleton, Cultist, EliteGuard, Boss
- NPC: Villager, Merchant, QuestNPC, Guard, Companion
- Quests: QuestGiver
- World: WorldPortal, LootChest, LootPickup, DemoInteractable
- Encounters: EncounterZone, EncounterSpawner

## Grenzen
Die erzeugten Blueprints erben zunächst das C++-Verhalten. **AnimGraph, Skeletal Meshes, Behavior Trees, Widget-Blueprints, individuelle Event-Graphs, Gegnerbalance, Quest-Content, Input-Assets und Gameplay-Verdrahtung sind noch nicht erstellt.** Der Editorlauf wurde hier nicht ausgeführt und kein UE-Build getestet. Ein vorhandenes Blueprint-Asset wird nicht überschrieben.

## Folgephase
Animationen und Eingaben verbinden, NPC- und Gegner-Varianten konfigurieren, AI-Behavior-Trees erstellen, UI-Widgets erzeugen, Quests und Encounter-Zonen anschließen, PIE-/Packaging-Tests.
