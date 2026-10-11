"""Offline preflight for V55; does not require Unreal Engine."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent

def validate():
    bindings=json.loads((ROOT/'Production/Levels/encounter_blueprint_bindings_v54.json').read_text())['encounters']
    profiles=json.loads((ROOT/'Production/Blueprints/combat_animation_profiles_v55.json').read_text())['profiles']
    names={p['pawn'] for p in profiles}
    assert len(profiles)==6 and len(names)==6
    assert len(bindings)==90 and len({r['encounter_id'] for r in bindings})==90
    assert len({r['map_name'] for r in bindings})==30
    for row in bindings:
        assert row['enemy_blueprint'] in names
        bp=row['enemy_blueprint']
        assert row['enemy_class_path']==f'/Game/Askir/Blueprints/Enemies/{bp}.{bp}_C'
        assert row['enemy_count']>=1
        assert row['completion_quest_id'] is None and row['completion_objective_id'] is None
    for p in profiles:
        assert p['pawn_generated_class']==p['pawn_asset']+'.'+p['pawn']+'_C'
        assert p['attack_distance_cm']>0 and p['attack_interval_seconds']>0
        assert p['animation_montage'] is None
    return {'encounters':len(bindings),'regions':30,'enemy_profiles':len(profiles),'status':'offline OK'}

if __name__=='__main__': print(validate())
