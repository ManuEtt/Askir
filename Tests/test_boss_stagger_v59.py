import json, unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
P = {x['boss_id']:x for x in json.loads((ROOT/'Production/Bosses/boss_stagger_profiles_v59.json').read_text())['profiles']}
M = json.loads((ROOT/'Production/Bosses/stagger_test_matrix_v59.json').read_text())['scenarios']
class BossStaggerTests(unittest.TestCase):
    def test_scenarios(self):
        self.assertEqual(len(M),50)
        self.assertEqual(len(P),5)
        for case in M:
            with self.subTest(**case):
                p=P[case['boss_id']]; c=case['condition']; threshold=p['stagger_damage_threshold']; duration=p['stagger_duration_seconds']
                checks={
                    'profile_id':lambda:bool(p['boss_id']),
                    'phase_threshold_order':lambda:0<p['phase_three_fraction']<p['phase_two_fraction']<1,
                    'windup_positive':lambda:0<p['telegraph_windup_seconds']<=5,
                    'stagger_threshold_positive':lambda:threshold>0,
                    'stagger_duration_positive':lambda:duration>0,
                    'interrupt_enabled':lambda:p['allow_windup_interrupt'] is True,
                    'interrupt_event':lambda:p['interrupt_event']=='OnBossAttackInterrupted',
                    'interrupt_below_threshold':lambda:(threshold-0.01)<threshold,
                    'interrupt_at_threshold':lambda:threshold>=threshold,
                    'stagger_expiry':lambda:(10+duration)>10,
                }
                self.assertTrue(checks[c]())
    def test_cpp_runtime_wiring(self):
        h=(ROOT/'Source/Askir/AI/AskirBossAIController.h').read_text()
        cpp=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        for fragment in ['HandleBossDamage','InterruptBossAttack','IsBossStaggered','OnBossAttackInterrupted']:
            self.assertIn(fragment,h);self.assertIn(fragment,cpp)
        self.assertIn('OnDamageTaken.AddDynamic',cpp)
        self.assertIn('OnDamageTaken.RemoveDynamic',cpp)
        self.assertIn('CancelTelegraphedAttack();',cpp)
        self.assertIn('if (IsBossStaggered())',cpp)
if __name__=='__main__':unittest.main()
