import json,unittest
from pathlib import Path
R=Path(__file__).resolve().parent.parent
class LootV69(unittest.TestCase):
 def test_fifty_scenarios(self):
  rows=json.loads((R/'Production/Bosses/boss_loot_verification_v69.json').read_text())['cases']
  self.assertEqual(len(rows),50)
  self.assertEqual([x['topic'] for x in rows],list(range(2201,2251)))
 def test_weighted_struct(self):
  h=(R/'Source/Askir/World/AskirEncounterSpawner.h').read_text()
  self.assertIn('FAskirEncounterLootEntry',h)
  self.assertIn('CompletionLootTable',h)
 def test_weight_guard(self):
  c=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('int64 TotalWeight=0',c)
  self.assertIn('Entry.Weight<=0',c)
 def test_reward_before_ledger(self):
  c=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertLess(c.index('Inventory->AddItem(SelectedReward,SelectedQuantity)'),c.index('Quests->MarkEncounterCompleted(ID)'))
 def test_duplicate_guard(self):
  c=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('Quests->HasCompletedEncounter(ID)',c)
