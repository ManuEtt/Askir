import json
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
D=json.loads((ROOT/'Production/Blueprints/blueprint_integration_v52.json').read_text())
def validate():
    entries=D['blueprints']; names=[e['name'] for e in entries]; paths=[e['asset_path'] for e in entries]
    assert len(entries)==24 and len(set(names))==24 and len(set(paths))==24
    assert all(e['native_path'].startswith('/Script/Askir.') for e in entries)
    for e in entries:
        assert (ROOT/'Source/Askir').exists()
        assert any(p.name==e['native_parent']+'.h' for p in (ROOT/'Source/Askir').rglob('*.h')),e['name']
    assert all(p['pawn'] in names and p['controller'] in names for p in D['ai_presets'])
    assert all(p['attack_distance']>0 and p['sight_range']>p['attack_distance'] and p['attack_interval']>0 for p in D['ai_presets'])
    return len(entries),len(D['ai_presets'])
if __name__=='__main__':print('PASS:',validate())
