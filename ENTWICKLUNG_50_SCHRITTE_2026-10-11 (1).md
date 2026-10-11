# Askir – 50 zusätzliche Entwicklungsschritte

**Basis:** AskirRPG_UE5_6_Weiterentwicklung_10_Themen_2026-10-11.zip

Die vorhandenen Projektdateien wurden übernommen. Zehn neue UE5-C++ ActorComponents mit jeweils fünf Blueprint-aufrufbaren Methoden sind hinzugekommen.

## 50 implementierte Schritte

### Survival
1. `Survival.AddWarmth(float Value)` – Zustandsänderung implementiert
2. `Survival.AddWetness(float Value)` – Zustandsänderung implementiert
3. `Survival.ApplyExposure(float Hours)` – Zustandsänderung implementiert
4. `Survival.DryOff(float Amount)` – Zustandsänderung implementiert
5. `Survival.RestAtFire(float Hours)` – Zustandsänderung implementiert

### Equipment
6. `Equipment.EquipArmor(float Rating)` – Zustandsänderung implementiert
7. `Equipment.EquipWeapon(float Rating)` – Zustandsänderung implementiert
8. `Equipment.DamageGear(float Amount)` – Zustandsänderung implementiert
9. `Equipment.RepairGear(float Amount)` – Zustandsänderung implementiert
10. `Equipment.UnequipAll()` – Zustandsänderung implementiert

### Stealth
11. `Stealth.MakeNoise(float Amount)` – Zustandsänderung implementiert
12. `Stealth.Crouch()` – Zustandsänderung implementiert
13. `Stealth.StandUp()` – Zustandsänderung implementiert
14. `Stealth.RaiseSuspicion(float Amount)` – Zustandsänderung implementiert
15. `Stealth.CalmDown(float Amount)` – Zustandsänderung implementiert

### Morale
16. `Morale.Encourage(float Amount)` – Zustandsänderung implementiert
17. `Morale.Intimidate(float Amount)` – Zustandsänderung implementiert
18. `Morale.RecoverFear(float Amount)` – Zustandsänderung implementiert
19. `Morale.TrainCourage(float Amount)` – Zustandsänderung implementiert
20. `Morale.BreakMorale(float Amount)` – Zustandsänderung implementiert

### Travel
21. `Travel.Walk(float Distance)` – Zustandsänderung implementiert
22. `Travel.ConsumeSupplies(float Amount)` – Zustandsänderung implementiert
23. `Travel.Rest(float Hours)` – Zustandsänderung implementiert
24. `Travel.Resupply(float Amount)` – Zustandsänderung implementiert
25. `Travel.FastTravel(float Distance)` – Zustandsänderung implementiert

### Trade
26. `Trade.EarnGold(int32 Amount)` – Zustandsänderung implementiert
27. `Trade.SpendGold(int32 Amount)` – Zustandsänderung implementiert
28. `Trade.RaiseFavor(int32 Amount)` – Zustandsänderung implementiert
29. `Trade.RecordTrade(int32 Value)` – Zustandsänderung implementiert
30. `Trade.PayFine(int32 Amount)` – Zustandsänderung implementiert

### Camp
31. `Camp.GatherFuel(float Amount)` – Zustandsänderung implementiert
32. `Camp.StockFood(float Amount)` – Zustandsänderung implementiert
33. `Camp.BuildShelter(float Amount)` – Zustandsänderung implementiert
34. `Camp.BurnFuel(float Hours)` – Zustandsänderung implementiert
35. `Camp.EatRations(float Amount)` – Zustandsänderung implementiert

### Companion
36. `Companion.Praise(float Amount)` – Zustandsänderung implementiert
37. `Companion.TrainTogether(float Hours)` – Zustandsänderung implementiert
38. `Companion.IncreaseStress(float Amount)` – Zustandsänderung implementiert
39. `Companion.Comfort(float Amount)` – Zustandsänderung implementiert
40. `Companion.LoseLoyalty(float Amount)` – Zustandsänderung implementiert

### Exploration
41. `Exploration.DiscoverSite()` – Zustandsänderung implementiert
42. `Exploration.ReadLore()` – Zustandsänderung implementiert
43. `Exploration.Survey(float Amount)` – Zustandsänderung implementiert
44. `Exploration.ResetSurvey()` – Zustandsänderung implementiert
45. `Exploration.StudyRuins(int32 Hours)` – Zustandsänderung implementiert

### Crafting
46. `Crafting.GatherMaterials(int32 Amount)` – Zustandsänderung implementiert
47. `Crafting.CraftItem(int32 Cost)` – Zustandsänderung implementiert
48. `Crafting.ImproveCraftSkill()` – Zustandsänderung implementiert
49. `Crafting.Salvage(int32 Amount)` – Zustandsänderung implementiert
50. `Crafting.ConsumeMaterials(int32 Amount)` – Zustandsänderung implementiert

## Integration und Grenzen

- Unreal Engine 5.6 öffnen, C++-Projektdateien generieren und mit Visual Studio bauen.
- Neue Komponenten in Character-/NPC-Blueprints hinzufügen und Methoden bei passenden Aktionen aufrufen.
- Die Komponenten sind derzeit unabhängig: UI, Quests, Animationen und Persistenz sind noch nicht integriert.
- `SaveGame`-Eigenschaften werden ohne explizite Snapshot-/Restore-Logik nicht automatisch gespeichert.
- Handel und Crafting sind vereinfachte Prototypen, keine vollständigen Transaktionen.
- Unreal-Kompilierung und Laufzeittests waren hier nicht möglich.
