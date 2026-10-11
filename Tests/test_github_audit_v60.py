"""Regression tests for confirmed V60/GitHub integration defects."""
import json
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
class GithubAuditTests(unittest.TestCase):
    def test_corrected_encounter_class_paths(self):
        from Tools.validate_encounter_bindings_v54 import validate
        self.assertEqual(validate(),90)
    def test_combat_settings_on_ai_controller(self):
        script=(ROOT/'Tools/configure_combat_profiles_v55.py').read_text()
        self.assertIn("path = row['controller_asset']",script)
        self.assertIn("'sight_range'",script)
        self.assertIn("'max_chase_from_home'",script)
        self.assertNotIn("'detection_radius':",script)
        self.assertNotIn("'max_chase_distance':",script)
    def test_boss_uses_distinct_controller_blueprint(self):
        script=(ROOT/'Tools/setup_boss_ai_v56.py').read_text()
        self.assertIn('BP_AI_AskirBossPhases',script)
        self.assertIn('is_child_of(native)',script)
        self.assertNotIn('"BP_AI_AskirBoss",',script)
    def test_all_combat_controller_paths(self):
        data=json.loads((ROOT/'Production/Blueprints/combat_animation_profiles_v55.json').read_text())
        self.assertEqual(len(data['profiles']),6)
        for p in data['profiles']:
            self.assertTrue(p['controller_asset'].startswith('/Game/Askir/Blueprints/AI/'))
        boss=next(p for p in data['profiles'] if p['pawn']=='BP_AskirBoss')
        self.assertTrue(boss['controller_asset'].endswith('BP_AI_AskirBossPhases'))
    def test_setup_is_preview_by_default(self):
        for name in ['configure_combat_profiles_v55.py','setup_boss_ai_v56.py']:
            self.assertIn('DRY_RUN = True',(ROOT/'Tools'/name).read_text())
if __name__=='__main__':unittest.main()
