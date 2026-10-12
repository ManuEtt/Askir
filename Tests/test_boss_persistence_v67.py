import json,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
class BossPersistenceV67(unittest.TestCase):
 def test_50_scenarios(self):
  rows=json.loads((ROOT/'Production/Bosses/boss_persistence_matrix_v67.json').read_text())['cases']
  self.assertEqual(len(rows),50)
  self.assertEqual([x['topic'] for x in rows],list(range(2101,2151)))
 def test_zone_persistent_guard(self):
  s=(ROOT/'Source/Askir/World/AskirEncounterZone.cpp').read_text()
  self.assertIn('SavedQuests->HasCompletedEncounter(EncounterID)',s)
 def test_entry_reward_toggle(self):
  h=(ROOT/'Source/Askir/World/AskirEncounterZone.h').read_text()
  self.assertIn('bAwardObjectiveOnEntry=true',h)
 def test_spawner_checks_id(self):
  s=(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('EncounterID!=Zone->EncounterID',s)
 def test_victory_reward(self):
  s=(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('!bZoneAwardsOnEntry',s)
  self.assertIn('Zone->ObjectiveID',s)
