import json
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PROFILES=json.loads((ROOT/'Production/Levels/encounter_profiles_v48.json').read_text(encoding='utf-8'))['profiles']
SOURCE=(ROOT/'Tools/integrate_encounter_actors_v49.py').read_text(encoding='utf-8')

class TestV49(unittest.TestCase):
    pass

def region_case(index):
    def test(self):
        profile=PROFILES[index]
        self.assertTrue(profile['map_name'].startswith('L'))
        self.assertEqual(len(profile['encounters']),3)
        self.assertEqual(len({e['encounter_id'] for e in profile['encounters']}),3)
        for encounter in profile['encounters']:
            self.assertEqual(len(encounter['position_cm']),3)
            self.assertGreaterEqual(encounter['enemy_count'],1)
            self.assertLessEqual(encounter['enemy_count'],32)
    return test
for i in range(30):
    setattr(TestV49,'test_region_%02d'%(i+1),region_case(i))

CONTRACTS = [
    ('dry_run','DRY_RUN = True'),
    ('requires_maps','does_asset_exist'),
    ('loads_maps','load_map'),
    ('saves_maps','save_current_level'),
    ('uses_zone_class','/Script/Askir.AskirEncounterZone'),
    ('uses_spawner_class','/Script/Askir.AskirEncounterSpawner'),
    ('zone_labels','V49_Zone_'),
    ('spawner_labels','V49_Spawner_'),
    ('idempotent','actors_by_label.get(label)'),
    ('class_collision_guard','Label collision with wrong class'),
    ('sets_encounter_id',"set_editor_property('encounter_id'"),
    ('sets_zone_reference',"set_editor_property('zone'"),
    ('sets_enemy_count',"set_editor_property('enemies_per_wave'"),
    ('sets_wave_count',"set_editor_property('wave_count'"),
    ('sets_spawn_radius',"set_editor_property('spawn_radius'"),
    ('optional_blueprints','blueprint_class_path'),
    ('missing_blueprint_guard','Enemy Blueprint class missing'),
    ('missing_maps_report','missing_maps'),
    ('no_editor_enemy_spawning','spawn_actor_from_class(cls'),
    ('explicit_run','if __name__ =='),
]
def contract_case(token):
    def test(self): self.assertIn(token,SOURCE)
    return test
for name,token in CONTRACTS:
    setattr(TestV49,'test_contract_'+name,contract_case(token))
if __name__=='__main__':unittest.main()
