import unittest
from Tools.audit_dornfeld_editor_contract_v75 import validate
class TestDornfeldEditorContractV75(unittest.TestCase):
    def test_actor_reflection_contract(self): self.assertEqual(validate(), [])
    def test_quest_id_is_assigned(self):
        from pathlib import Path
        script=(Path(__file__).resolve().parents[1]/'Tools/build_dornfeld_playground_v74.py').read_text()
        self.assertIn("set_editor_property('quest_id', 'DORNFELD_BRUNNEN')",script)
    def test_guardian_objective(self):
        from pathlib import Path
        script=(Path(__file__).resolve().parents[1]/'Tools/build_dornfeld_playground_v74.py').read_text()
        self.assertIn("set_editor_property('defeat_objective_id', 'DEFEAT_GUARDIANS')",script)
