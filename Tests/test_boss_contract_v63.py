import unittest,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(ROOT/'Tools'))
from validate_boss_contract_v63 import validate
class TestBossContractV63(unittest.TestCase):
    def test_contract(self):self.assertTrue(validate())
    def test_delegate_cleanup_before_super_possess(self):
        src=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        block=src.split('void AAskirBossAIController::OnPossess(APawn* InPawn)',1)[1].split('void AAskirBossAIController::Tick',1)[0]
        self.assertLess(block.index('OnDamageTaken.RemoveDynamic'),block.index('Super::OnPossess(InPawn)'))
    def test_cancel_clears_notify(self):
        src=(ROOT/'Source/Askir/Characters/AskirCharacter.cpp').read_text()
        block=src.split('void AAskirCharacter::CancelPendingAttack()',1)[1].split('void AAskirCharacter::Dodge()',1)[0]
        self.assertIn('bUseAnimationNotifyForImpact=false;',block)
    def test_telegraph_event_always_emitted(self):
        src=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
        block=src.split('void AAskirBossAIController::TriggerAttackTelegraph()',1)[1].split('void AAskirBossAIController::OnUnPossess()',1)[0]
        self.assertNotIn('if (Now - LastTelegraphTime',block)
        self.assertIn('OnBossAttackTelegraph(',block)
if __name__=='__main__':unittest.main()
