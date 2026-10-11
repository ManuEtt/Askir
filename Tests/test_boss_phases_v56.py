import unittest
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "Tools"))
from boss_phase_model_v56 import next_phase, attack_interval

class TestBossPhases(unittest.TestCase):
    def test_50_phase_scenarios(self):
        # 50 deterministic combinations across health levels and previous phases.
        health_values = [1.0, .9, .8, .7, .66, .65, .5, .34, .33, .32, .2, .1, 0.0, .99, .4, .75, .15]
        scenarios = [(phase, health) for phase in (1, 2, 3) for health in health_values][:50]
        self.assertEqual(len(scenarios), 50)
        for phase, health in scenarios:
            with self.subTest(phase=phase, health=health):
                expected = max(phase, 3 if health <= .33 else 2 if health <= .66 else 1)
                self.assertEqual(next_phase(phase, health), expected)
    def test_attack_speed(self):
        self.assertAlmostEqual(attack_interval(1.4, 1.4), 1.0)
    def test_invalid_thresholds(self):
        with self.assertRaises(ValueError): next_phase(1, .5, .3, .6)
    def test_native_files(self):
        root = Path(__file__).resolve().parents[1]
        self.assertTrue((root / "Source/Askir/AI/AskirBossAIController.cpp").exists())
        self.assertTrue((root / "Production/Blueprints/boss_phase_profiles_v56.json").exists())
if __name__ == "__main__": unittest.main()
