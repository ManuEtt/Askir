import copy,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'Tools'))
from encounter_runtime_v48 import Encounter,Phase
from validate_encounters_v48 import validate
LEVELS=json.loads((ROOT/'Production/Levels/level_designs_v42.json').read_text())['levels']
PROFILES=json.loads((ROOT/'Production/Levels/encounter_profiles_v48.json').read_text())['profiles']
class EncounterTests(unittest.TestCase):
 def test_all_30_regions(self):self.assertEqual(len(PROFILES),30)
 def test_all_90_encounters(self):self.assertEqual(sum(len(p['encounters']) for p in PROFILES),90)
 def test_profiles_valid(self):self.assertEqual(validate(LEVELS,PROFILES),[])
 def test_dormant_outside_radius(self):
  e=Encounter('test',2);self.assertFalse(e.activate(651,650))
 def test_activate_inside_radius(self):
  e=Encounter('test',2);self.assertTrue(e.activate(650,650));self.assertEqual(e.remaining,2)
 def test_gate_blocks_activation(self):self.assertFalse(Encounter('test',1).activate(1,650,False))
 def test_cannot_activate_twice(self):
  e=Encounter('test',1);e.activate(0,650);self.assertFalse(e.activate(0,650))
 def test_kills_clear(self):
  e=Encounter('test',2);e.activate(0,650);e.enemy_defeated();self.assertEqual(e.phase,Phase.ACTIVE);e.enemy_defeated();self.assertEqual(e.phase,Phase.CLEARED)
 def test_kill_dormant_ignored(self):self.assertFalse(Encounter('test',1).enemy_defeated())
 def test_kill_after_clear_ignored(self):
  e=Encounter('test',1);e.activate(0,650);e.enemy_defeated();self.assertFalse(e.enemy_defeated())
 def test_reset(self):
  e=Encounter('test',1);e.activate(0,650);e.reset_on_map_reload();self.assertEqual(e.phase,Phase.DORMANT)
 def test_snapshot_roundtrip(self):
  e=Encounter('test',2);e.activate(0,650);e.enemy_defeated();f=Encounter('test',2);f.restore(e.snapshot());self.assertEqual(f.snapshot(),e.snapshot())
 def test_snapshot_wrong_id(self):
  with self.assertRaises(ValueError):Encounter('test',1).restore({'id':'wrong','phase':'active','remaining':1})
 def test_snapshot_bad_remaining(self):
  with self.assertRaises(ValueError):Encounter('test',1).restore({'id':'test','phase':'active','remaining':5})
 def test_snapshot_bad_phase(self):
  with self.assertRaises(ValueError):Encounter('test',1).restore({'id':'test','phase':'invalid','remaining':1})
 def test_snapshot_inconsistent(self):
  with self.assertRaises(ValueError):Encounter('test',1).restore({'id':'test','phase':'dormant','remaining':1})
 def test_duplicate_profile(self):
  p=copy.deepcopy(PROFILES);p.append(copy.deepcopy(p[0]));self.assertTrue(validate(LEVELS,p))
 def test_wrong_position(self):
  p=copy.deepcopy(PROFILES);p[0]['encounters'][0]['position_cm'][0]+=1;self.assertTrue(validate(LEVELS,p))
 def test_missing_encounter(self):
  p=copy.deepcopy(PROFILES);p[0]['encounters'].pop();self.assertTrue(validate(LEVELS,p))
 def test_invalid_enemy_count(self):
  p=copy.deepcopy(PROFILES);p[0]['encounters'][0]['enemy_count']=0;self.assertTrue(validate(LEVELS,p))
if __name__=='__main__':unittest.main()
