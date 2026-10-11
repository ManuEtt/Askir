import unittest
from pathlib import Path
R=Path(__file__).resolve().parents[1]
class BossWindupChecks(unittest.TestCase):
    def test_base_hook_declared(self): self.assertIn('virtual bool TryStartAttack', (R/'Source/Askir/AI/AskirEnemyAIController.h').read_text())
    def test_base_hook_used(self): self.assertIn('if (TryStartAttack(ControlledEnemy, Target))', (R/'Source/Askir/AI/AskirEnemyAIController.cpp').read_text())
    def test_timer(self): self.assertIn('SetTimer(TelegraphTimer', (R/'Source/Askir/AI/AskirBossAIController.cpp').read_text())
    def test_visibility(self): self.assertIn('LineTraceSingleByChannel', (R/'Source/Askir/AI/AskirBossAIController.cpp').read_text())
    def test_range(self): self.assertIn('DistSquared', (R/'Source/Askir/AI/AskirBossAIController.cpp').read_text())
    def test_cleanup(self): self.assertIn('ClearTimer(TelegraphTimer)', (R/'Source/Askir/AI/AskirBossAIController.cpp').read_text())
    def test_no_direct_damage(self): self.assertNotIn('ApplyDamage(', (R/'Source/Askir/AI/AskirBossAIController.cpp').read_text())
if __name__=='__main__': unittest.main()
