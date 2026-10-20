"""Source/data regression checks; no Unreal runtime claims."""
import importlib.util
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
class MagicContracts(unittest.TestCase):
    def test_canonical_catalog_matches_actual_generator(self):
        spec=importlib.util.spec_from_file_location('spellgen',ROOT/'Tools/generate_spell_catalog_v78.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        self.assertEqual((ROOT/'Source/Askir/Magic/AskirSpellCatalog.h').read_text(),module.render())
    def test_restore_magic_preflight_precedes_mutation(self):
        text=(ROOT/'Source/Askir/Game/AskirGameMode.cpp').read_text().split('bool AAskirGameMode::RestoreSnapshot',1)[1]
        self.assertLess(text.index('CanRestoreSnapshot(RestoredMagic)'),text.index('OperationGuard'))
        self.assertLess(text.index('OperationGuard'),text.index('It->Destroy()'))
        self.assertIn('Save->SaveFormatVersion>=4?',text)
    def test_all_gameplay_entry_actions_respect_menu(self):
        text=(ROOT/'Source/Askir/Characters/AskirCharacter.cpp').read_text()
        for method in ('InputJump','StartSprint','Dodge','StartBlock','InputCast','InputCycleSpell'):
            body=text.split('void AAskirCharacter::'+method+'(',1)[1].split('\nvoid AAskirCharacter::',1)[0]
            self.assertIn('CanUseGameplayInput()',body,method)
        controller=(ROOT/'Source/Askir/Game/AskirPlayerController.cpp').read_text();self.assertIn('Player->CancelGameplayActions()',controller)
    def test_projectile_credits_instigator_and_sweeps(self):
        text=(ROOT/'Source/Askir/Magic/AskirSpellProjectile.cpp').read_text()
        self.assertIn('SetInstigator(Caster)',text);self.assertIn('SweepSingleByChannel',text);self.assertIn('ApplySpellHit(GetInstigator()',text)
    # V82 civilian protection is exercised against the shared production rules in
    # test_combat_target_rules_v82.cpp and registered native integration tests.
    # A substring check against the former inline implementation adds no coverage.
if __name__=='__main__':unittest.main()
