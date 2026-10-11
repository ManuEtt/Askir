"""Offline validation of the V54 encounter/Blueprint binding contract."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent

def validate():
    source=json.loads((ROOT/'Production/Levels/encounter_profiles_v48.json').read_text(encoding='utf-8'))['profiles']
    ai=json.loads((ROOT/'Production/Blueprints/ai_profiles_v53.json').read_text(encoding='utf-8'))['profiles']
    rows=json.loads((ROOT/'Production/Levels/encounter_blueprint_bindings_v54.json').read_text(encoding='utf-8'))['encounters']
    expected={(p['map_name'],e['encounter_id']):e for p in source for e in p['encounters']}
    allowed={p['pawn_path'] + '.' + p['pawn_path'].rsplit('/', 1)[-1] + '_C' for p in ai}
    assert len(expected)==len(rows)==90
    assert len({r['encounter_id'] for r in rows})==90
    for r in rows:
        key=(r['map_name'],r['encounter_id'])
        assert key in expected,key
        assert r['enemy_archetype']==expected[key]['enemy_archetype']
        assert r['enemy_count']==expected[key]['enemy_count']
        assert r['enemy_class_path'] in allowed
        assert r['completion_quest_id'] is None and r['completion_objective_id'] is None
    return len(rows)
if __name__=='__main__':print('Validated bindings:',validate())
