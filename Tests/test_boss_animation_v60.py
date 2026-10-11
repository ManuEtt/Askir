import unittest, json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DATA=json.loads((ROOT/'Production/Bosses/boss_animation_profiles_v60.json').read_text())['profiles']
class BossAnimationTests(unittest.TestCase):
 def test_count(self): self.assertEqual(len(DATA),5)
 def test_unique_ids(self): self.assertEqual(len({x['boss_id'] for x in DATA}),5)
 def test_unreal_assets_not_fabricated(self): self.assertTrue(all(x['attack_montage_asset'] is None for x in DATA))
 def test_notify_disabled_without_asset(self): self.assertTrue(all(not x['impact_notify_enabled'] for x in DATA))
 def test_source_uses_existing_damage_path(self):
  self.assertIn('Super::TryStartAttack', (ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text())
 def test_source_guards_single_impact(self):
  self.assertIn('if(!bPendingAttack)return', (ROOT/'Source/Askir/Characters/AskirCharacter.cpp').read_text())
for i in range(5):
 for field in ('telegraph_windup_seconds','stagger_seconds','montage_play_rate','attack_montage_asset','stagger_montage_asset','impact_notify_enabled','animation_status','boss_id'):
  def check(self, i=i, field=field):
   value=DATA[i][field]
   self.assertIsNotNone(value) if field not in ('attack_montage_asset','stagger_montage_asset') else self.assertIsNone(value)
  setattr(BossAnimationTests,'test_profile_%d_%s'%(i,field),check)
for i in range(4):
 def check(self,i=i):
  self.assertGreater(DATA[i]['montage_play_rate'],0)
 setattr(BossAnimationTests,'test_rate_%d'%i,check)
if __name__=='__main__': unittest.main()
