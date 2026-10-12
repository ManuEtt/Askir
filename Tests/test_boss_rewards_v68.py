import json,unittest
from pathlib import Path
R=Path(__file__).resolve().parent.parent
class RewardsV68(unittest.TestCase):
 def test_scenarios(self):
  x=json.loads((R/'Production/Bosses/boss_reward_scenarios_v68.json').read_text())['scenarios']
  self.assertEqual(len(x),50)
  self.assertEqual([r['topic'] for r in x],list(range(2151,2201)))
 def test_config(self):
  h=(R/'Source/Askir/World/AskirEncounterSpawner.h').read_text()
  self.assertIn('CompletionRewardItem',h)
  self.assertIn('CompletionRewardQuantity',h)
 def test_reward_guard(self):
  c=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('if(bHasReward && !IsValid(Inventory))',c)
  self.assertIn('if(bHasReward && !Inventory->AddItem(SelectedReward,SelectedQuantity))',c)
 def test_persistence(self):
  h=(R/'Source/Askir/Save/AskirSaveGame.h').read_text()
  self.assertIn('SavedCompletedEncounters',h)
  self.assertIn('SavedInventory',h)
