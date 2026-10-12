import json,unittest
from pathlib import Path
R=Path(__file__).resolve().parent.parent
class LootV70(unittest.TestCase):
 def test_fifty_scenarios(self):
  cases=json.loads((R/'Production/Bosses/boss_loot_verification_v70.json').read_text())['cases']
  self.assertEqual(len(cases),50)
  self.assertEqual([c['topic'] for c in cases],list(range(2251,2301)))
 def test_stable_seed_for_persistent_encounter(self):
  s=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('FCrc::StrCrc32(*RewardEncounterID.ToString())',s)
  self.assertIn('RewardEncounterID.IsNone()',s)
 def test_draw_is_bounded(self):
  s=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('FMath::Min(TotalWeight-1,',s)
 def test_persistent_ledger_required(self):
  s=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('!IsValid(Player->FindComponentByClass<UAskirQuestComponent>())',s)
 def test_completion_releases_player_reference(self):
  s=(R/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('OnEncounterFinished.Broadcast(ActivePlayer.Get(),IsValid(Zone)?Zone->EncounterID:NAME_None);\n ActivePlayer=nullptr;',s)
