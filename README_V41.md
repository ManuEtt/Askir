# Askir RPG V41

Version 41 ergänzt inventargestütztes Ausrüsten und eine Konsistenzprüfung nach dem Laden sowie vor dem Speichern.

- `UAskirEquipmentComponent::TryEquipInventoryItem`: prüft Item-Besitz, Item-Typ und ID.
- `ReconcileWithInventory`: entfernt Ausrüstung, deren Item im Inventar fehlt.
- `AAskirGameMode`: prüft Ausrüstung vor dem Speichern und nach Inventar-Restore.

**Grenzen:** Keine Unreal-Kompilierung; Rüstungsassets besitzen noch kein ArmorRating-Feld, daher wird ein neuer Rüstungsgegenstand vorerst mit 0 Rüstungswert ausgerüstet. Keine vollständige Level-/Windows-Build-Version.
