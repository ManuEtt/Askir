# Askir RPG – 50 neue Integrationsschritte

Stand: 11.10.2026 | Basis: vorherige 50-Schritte-ZIP

Diese Version ergänzt die vorhandene Projektstruktur. Die zehn zuvor erstellten Systeme werden als Komponenten am Spielercharakter instanziiert.

## 50 neue integrierte Aktionen

1. **Survival** – `EatFromCamp(float Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
2. **Survival** – `DrinkAndRecover(float Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
3. **Survival** – `WarmAtCamp(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
4. **Survival** – `ColdTravel(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
5. **Survival** – `DryInShelter(float Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
6. **Equipment** – `DamageEquipmentOnHit(float Damage)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
7. **Equipment** – `RepairUsingMaterials(int32 MaterialsCost)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
8. **Equipment** – `EquipArmorForProtection(float Rating)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
9. **Equipment** – `EquipWeaponForDamage(float Rating)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
10. **Equipment** – `SellDamagedGear(int32 GoldValue)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
11. **Stealth** – `CrouchAndQuiet()`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
12. **Stealth** – `RaiseAlarm(float Severity)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
13. **Stealth** – `SneakTravel(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
14. **Stealth** – `HideAtCamp()`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
15. **Stealth** – `RecoverAfterAlert(float Minutes)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
16. **Morale** – `EncourageCompanion(float Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
17. **Morale** – `FearFromAmbush(float Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
18. **Morale** – `RestMorale(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
19. **Morale** – `TrainWithCompanion(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
20. **Morale** – `LoseMoraleFromHunger(float Hunger)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
21. **Travel** – `TravelWithSupplies(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
22. **Travel** – `BuyTravelSupplies(int32 GoldCost)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
23. **Travel** – `RestAtCamp(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
24. **Travel** – `FastTravelWithCost(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
25. **Travel** – `SurveyWhileTraveling(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
26. **Trade** – `SellCraftedItem(int32 Price)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
27. **Trade** – `BuyMaterials(int32 Count)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
28. **Trade** – `BuyCampFuel(int32 Count)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
29. **Trade** – `BuyCampFood(int32 Count)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
30. **Trade** – `RewardExploration(int32 GoldReward)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
31. **Camp** – `BuildShelterFromMaterials(int32 Count)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
32. **Camp** – `CollectWoodWhileTraveling(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
33. **Camp** – `CookRations(float Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
34. **Camp** – `ShelterCompanion(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
35. **Camp** – `BreakCamp()`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
36. **Companion** – `RewardCompanion(int32 Gold)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
37. **Companion** – `TrainCompanion(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
38. **Companion** – `ComfortCompanionAtFire(float Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
39. **Companion** – `TravelTogether(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
40. **Companion** – `CompanionUnderThreat(float Threat)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
41. **Exploration** – `DiscoverAndReward(int32 Experience)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
42. **Exploration** – `StudyAndLearn(int32 Hours)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
43. **Exploration** – `SurveyWithSupplies(float Distance)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
44. **Exploration** – `GatherAtDiscovery(int32 Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
45. **Exploration** – `ShareLoreWithCompanion(int32 Lore)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
46. **Crafting** – `CraftAndEarnExperience(int32 Cost)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
47. **Crafting** – `SalvageForCamp(int32 Amount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
48. **Crafting** – `TrainCraftingAtCamp()`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
49. **Crafting** – `CraftRepairKit(int32 Cost)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.
50. **Crafting** – `TradeCraftForSupplies(int32 SupplyAmount)`: verbindet mindestens zwei bestehende Spielsysteme bzw. koppelt eine Aktion an Ressourcen oder Fortschritt.

## Weitere Integration
- Alle 10 früheren Gameplay-Komponenten werden direkt im Konstruktor von `AAskirCharacter` erstellt.
- Die vorhandene Needs-Komponente ist ebenfalls dem Spielercharakter zugeordnet.
- Der bestehende GameMode sichert und lädt nun auch den Progression-Snapshot (Level, XP, Attribute, Skills, Perks, Ruf).

## Grenzen und offene Punkte
- Die 50 Aktionen sind C++- und Blueprint-aufrufbare Integrationspunkte, keine fertigen UI- oder Animationsabläufe.
- Vorhandene SaveGame-Dateien besitzen keine Versionierung; Migration/Kompatibilität älterer Spielstände muss getestet werden.
- Weitere Komponentenwerte (Survival, Trade, Camp etc.) sind noch nicht Teil des zentralen SaveGame.
- Keine Unreal-Engine-5.6-Kompilierung, Editorprüfung oder Spieltests durchgeführt.
- Der vorhandene Projektcode kann weitere Fehler enthalten, die erst beim Unreal-Build sichtbar werden.

## Nächster Prüfschritt
In UE 5.6 Projekt öffnen, C++ bauen, Character-Blueprint laden, integrierte Aktionen aufrufen und Save/Load prüfen.
