import json
import unittest
from pathlib import Path
from Tools.dornfeld_mission_preflight_v73 import MISSION, validate_mission

class DornfeldContractTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads(MISSION.read_text(encoding='utf-8'))

    def test_current_mission_contract(self):
        self.assertEqual(validate_mission(self.data), [])

    def test_duplicate_stage_rejected(self):
        self.data['stages'].append(dict(self.data['stages'][0]))
        self.assertTrue(validate_mission(self.data))

    def test_reward_regression_rejected(self):
        self.data['reward']['xp'] = 0
        self.assertTrue(validate_mission(self.data))

    def test_missing_guardian_rejected(self):
        self.data['stages'][2]['count'] = 1
        self.assertTrue(validate_mission(self.data))

    def test_stage_reorder_rejected(self):
        self.data['stages'][1], self.data['stages'][2] = self.data['stages'][2], self.data['stages'][1]
        self.assertTrue(validate_mission(self.data))

if __name__ == '__main__': unittest.main()
