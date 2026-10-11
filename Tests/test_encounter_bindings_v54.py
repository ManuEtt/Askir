import json,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
rows=json.loads((ROOT/'Production/Levels/encounter_blueprint_bindings_v54.json').read_text())['encounters']
source=json.loads((ROOT/'Production/Levels/encounter_profiles_v48.json').read_text())['profiles']
expected={(p['map_name'],e['encounter_id']):e for p in source for e in p['encounters']}
class BindingTests(unittest.TestCase):
    def test_coverage(self):self.assertEqual(len(rows),90)
    def test_unique_ids(self):self.assertEqual(len(set(x['encounter_id'] for x in rows)),90)
    def test_maps(self):self.assertEqual(len(set(x['map_name'] for x in rows)),30)
    def test_known_classes(self):
        names={'BP_Bandit','BP_Wolf','BP_Skeleton','BP_Cultist','BP_EliteGuard','BP_AskirBoss'}
        self.assertEqual({x['enemy_blueprint'] for x in rows},names)
    def test_no_unverified_quest_ids(self):self.assertTrue(all(x['completion_quest_id'] is None for x in rows))
    def test_editor_script_safe_default(self):
        text=(ROOT/'Tools/bind_encounter_blueprints_v54.py').read_text()
        self.assertIn('DRY_RUN = True',text)
        self.assertIn('V49_Spawner_',text)
    def test_validator(self):
        import sys
        sys.path.insert(0,str(ROOT/'Tools'))
        from validate_encounter_bindings_v54 import validate
        self.assertEqual(validate(),90)

def make_map_test(map_name):
    def check(self):
        matched=[r for r in rows if r['map_name']==map_name]
        self.assertEqual(len(matched),3)
        for r in matched:
            self.assertEqual(r['enemy_archetype'],expected[(map_name,r['encounter_id'])]['enemy_archetype'])
            self.assertTrue(r['enemy_class_path'].endswith('_C'))
    return check
for name in sorted({r['map_name'] for r in rows}):
    setattr(BindingTests,'test_region_'+name,make_map_test(name))

def make_archetype_test(archetype):
    def check(self):
        matches=[r for r in rows if r['enemy_archetype']==archetype]
        self.assertTrue(matches)
        self.assertEqual(len({r['enemy_class_path'] for r in matches}),1)
    return check
for name in sorted({r['enemy_archetype'] for r in rows}):
    setattr(BindingTests,'test_archetype_'+name,make_archetype_test(name))
