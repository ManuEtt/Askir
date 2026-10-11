import unittest, json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
from Tools.validate_combat_pipeline_v55 import validate
B=json.loads((ROOT/'Production/Levels/encounter_blueprint_bindings_v54.json').read_text())['encounters']
P=json.loads((ROOT/'Production/Blueprints/combat_animation_profiles_v55.json').read_text())['profiles']
class TestCombatPipeline(unittest.TestCase):
    def test_preflight(self): self.assertEqual(validate()['status'],'offline OK')
    def test_unique_encounters(self): self.assertEqual(len(set(x['encounter_id'] for x in B)),90)
    def test_all_maps(self): self.assertEqual(len(set(x['map_name'] for x in B)),30)
    def test_all_enemy_types(self): self.assertEqual(set(x['enemy_blueprint'] for x in B),set(x['pawn'] for x in P))
    def test_quest_ids_not_fabricated(self): self.assertTrue(all(x['completion_quest_id'] is None for x in B))
    def test_animation_assets_not_fabricated(self): self.assertTrue(all(x['animation_montage'] is None for x in P))
    def test_attack_contract(self): self.assertTrue(all('OnAttackLanded' in x['events'] for x in P))
    def test_controller_profiles(self): self.assertEqual(len(set(x['controller_asset'] for x in P)),6)
    def test_positive_attack_values(self): self.assertTrue(all(x['attack_distance_cm']>0 and x['attack_interval_seconds']>0 for x in P))
    def test_class_path_normalized(self): self.assertTrue(all('.'+x['enemy_blueprint']+'_C' in x['enemy_class_path'] for x in B))

# Thirty region-specific checks plus ten independent profile checks: 50 checks total.
for i in range(1,31):
    def check(self,n=i):
        subset=[x for x in B if x['map_name'].startswith(f'L{n:02d}_')]
        self.assertEqual(len(subset),3)
        self.assertEqual(len(set(x['encounter_id'] for x in subset)),3)
    setattr(TestCombatPipeline,f'test_region_{i:02d}',check)
for i in range(10):
    def check(self,n=i):
        p=P[n%len(P)]
        self.assertTrue(p['pawn_generated_class'].endswith('.'+p['pawn']+'_C'))
        self.assertGreater(p['decision_interval_seconds'],0)
    setattr(TestCombatPipeline,f'test_profile_{i:02d}',check)
