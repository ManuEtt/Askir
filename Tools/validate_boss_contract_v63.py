"""Offline source/manifest contract checks; does not compile Unreal C++."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
def validate():
    boss=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
    character=(ROOT/'Source/Askir/Characters/AskirCharacter.cpp').read_text()
    ai=(ROOT/'Source/Askir/AI/AskirEnemyAIController.cpp').read_text()
    assert 'OnDamageTaken.RemoveDynamic' in boss
    assert 'LastTelegraphTime = Now;' in boss
    assert 'if (Now - LastTelegraphTime <' not in boss
    assert 'bRequested && Enemy->HasPendingAttack()' in boss
    assert 'return Enemy->HasPendingAttack();' in ai
    assert 'void AAskirCharacter::CancelPendingAttack()' in character
    body=character.split('void AAskirCharacter::CancelPendingAttack()',1)[1].split('void AAskirCharacter::Dodge()',1)[0]
    assert 'bUseAnimationNotifyForImpact=false;' in body
    rows=json.loads((ROOT/'Production/Bosses/boss_regression_matrix_v63.json').read_text())['cases']
    assert len(rows)==50 and len({r['topic'] for r in rows})==50
    return True
if __name__=='__main__':
    validate()
    print('V63 source contracts: PASS (offline, not UE compile)')
