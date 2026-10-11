import json,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
class EncounterLifecycleV65(unittest.TestCase):
 def test_matrix(self):
  rows=json.loads((ROOT/'Production/Encounters/encounter_regression_v65.json').read_text())['cases']
  self.assertEqual(len(rows),50)
  self.assertEqual([r['topic'] for r in rows],list(range(2001,2051)))
 def test_default_ai_controller(self):
  self.assertIn('Enemy->SpawnDefaultController();',(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text())
 def test_no_false_wave_completion(self):
  code=(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('if(SpawnWave())return;',code)
  self.assertIn('void AAskirEncounterSpawner::FinishEncounter()',code)
 def test_cleanup(self):
  code=(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('void AAskirEncounterSpawner::EndPlay(',code)
  self.assertIn('void UAskirEncounterEnemyTracker::OnComponentDestroyed(',code)
 def test_reentrancy(self):
  self.assertIn('TGuardValue<bool> TransitionGuard',(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text())
