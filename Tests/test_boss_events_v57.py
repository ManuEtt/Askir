import json, pathlib, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class BossEventMatrix(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((ROOT/'Production/Bosses/boss_encounter_profiles_v57.json').read_text())['profiles']
        cls.header=(ROOT/'Source/Askir/AI/AskirBossAIController.h').read_text()
        cls.impl=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
    def test_matrix(self):
        self.assertEqual(len(self.data),5)
        for p in self.data:
            with self.subTest(boss=p['boss_id']):
                checks=[p['phase_three_fraction']<p['phase_two_fraction'],p['phase_three_fraction']>0,p['phase_two_fraction']<1,p['telegraph_windup_seconds']>0,p['telegraph_windup_seconds']<=5,p['phase_event']=='OnPhaseTransition',p['attack_event']=='OnBossAttackTelegraph',bool(p['boss_id']),p['implementation']=='data_and_cpp_hooks_only',p['boss_id'].isalnum()]
                for i,ok in enumerate(checks):
                    with self.subTest(rule=i):self.assertTrue(ok)
    def test_cpp_hooks(self):
        for symbol in ('OnPhaseTransition.Broadcast','OnBossPhaseChanged','TriggerAttackTelegraph','OnBossAttackTelegraph'):
            self.assertIn(symbol,self.impl)
        self.assertIn('DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams',self.header)
if __name__=='__main__':unittest.main()
