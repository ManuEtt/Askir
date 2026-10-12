import ast
import pathlib
import unittest

SCRIPT = pathlib.Path(__file__).resolve().parents[1] / 'Tools' / 'build_dornfeld_playground_v74.py'

class DornfeldEditorScriptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tree = ast.parse(SCRIPT.read_text(encoding='utf-8'))
        cls.source = SCRIPT.read_text(encoding='utf-8')
    def test_script_parses(self):
        self.assertIsInstance(self.tree, ast.Module)
    def test_safe_preview_default(self):
        self.assertIn('PREVIEW_ONLY = True', self.source)
    def test_never_overwrites_map(self):
        self.assertIn('Refusing to overwrite existing map', self.source)
    def test_real_actor_classes(self):
        for cls in ('AskirNPC', 'AskirQuestTrigger', 'AskirLootChest'):
            self.assertIn(cls, self.source)
    def test_saves_actual_editor_level(self):
        self.assertIn('save_current_level()', self.source)
    def test_no_unverified_playability_claim(self):
        self.assertIn('remain unverified', self.source)
