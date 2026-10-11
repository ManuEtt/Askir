import unittest,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class BossCombatV61(unittest.TestCase):
 def test_matrix(self):
  rows=json.loads((ROOT/'Production/Bosses/boss_combat_verification_v61.json').read_text())['scenarios']
  self.assertEqual(len(rows),50)
  self.assertEqual([x['topic'] for x in rows],list(range(1801,1851)))
  self.assertEqual(len({(x['boss_id'],x['check']) for x in rows}),50)
 def test_watchdog_no_phantom_damage(self):
  s=(ROOT/'Source/Askir/Characters/AskirCharacter.cpp').read_text()
  self.assertIn('void AAskirCharacter::ExpireUnresolvedAnimationAttack()',s)
  self.assertIn('CancelPendingAttack();\n    OnAttackMissed(bHeavy);',s)
  self.assertIn('bUseAnimationNotifyForImpact ? &AAskirCharacter::ExpireUnresolvedAnimationAttack',s)
 def test_committed_attack_stagger(self):
  s=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
  self.assertIn('bAllowCommittedAttackInterrupt && BossPawn->HasPendingAttack()',s)
  self.assertIn('BossPawn->AbortPendingAttack();',s)
 def test_profile_asset_refs_are_not_invented(self):
  rows=json.loads((ROOT/'Production/Bosses/boss_animation_profiles_v60.json').read_text())['profiles']
  self.assertEqual(len(rows),5)
  for r in rows:
   self.assertIsNone(r['attack_montage_asset'])
   self.assertIsNone(r['stagger_montage_asset'])
if __name__=='__main__': unittest.main()
