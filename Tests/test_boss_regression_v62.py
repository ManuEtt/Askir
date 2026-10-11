import json,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
class BossRegressionV62(unittest.TestCase):
    def test_fifty_unique_scenarios(self):
        rows=json.loads((ROOT/'Production/Bosses/boss_regression_matrix_v62.json').read_text())['cases']
        self.assertEqual(len(rows),50)
        self.assertEqual([r['topic'] for r in rows],list(range(1851,1901)))
        self.assertEqual(len({(r['boss'],r['scenario']) for r in rows}),50)
    def test_no_false_claim_of_pie(self):
        rows=json.loads((ROOT/'Production/Bosses/boss_regression_matrix_v62.json').read_text())['cases']
        self.assertTrue(all(r['status']=='offline_contract_only' for r in rows))
    def test_attack_commit_requires_pending_hit(self):
        code=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        self.assertIn('bRequested && Enemy->HasPendingAttack()',code)
        self.assertIn('if (!bCommitted) Enemy->SetImpactFromAnimationNotify(false)',code)
    def test_base_ai_cooldown_checks_actual_attack(self):
        code=(ROOT/'Source/Askir/AI/AskirEnemyAIController.cpp').read_text()
        self.assertIn('return Enemy->HasPendingAttack();',code)
if __name__=='__main__':unittest.main()
