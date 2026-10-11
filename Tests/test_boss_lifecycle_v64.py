import unittest,json
from pathlib import Path
R=Path(__file__).resolve().parent.parent
class BossLifecycleV64(unittest.TestCase):
    def test_phase_baseline_from_blueprint_defaults(self):
        s=(R/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        self.assertIn('GetDefaultObject<AAskirBossAIController>()',s)
        self.assertIn('AttackInterval = Defaults->AttackInterval;',s)
        self.assertIn('AttackDistance = Defaults->AttackDistance;',s)
    def test_dead_boss_aborts_pending_attack(self):
        s=(R/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        self.assertIn('BossPawn->AbortPendingAttack();\n        return;',s)
    def test_unpossess_resets_impact_mode(self):
        s=(R/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        self.assertIn('BossPawn->SetImpactFromAnimationNotify(false);',s)
    def test_scenarios(self):
        d=json.loads((R/'Production/Bosses/boss_regression_matrix_v64.json').read_text())
        self.assertEqual(len(d['scenarios']),50)
        self.assertEqual([x['topic'] for x in d['scenarios']],list(range(1951,2001)))
    def test_readiness_audit(self):
        from importlib.util import spec_from_file_location,module_from_spec
        p=R/'Tools/audit_build_readiness_v64.py';spec=spec_from_file_location('audit64',p);mod=module_from_spec(spec);spec.loader.exec_module(mod)
        self.assertEqual(mod.audit(),[])
