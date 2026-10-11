"""Offline structural validation of Blueprint manifest (not an Unreal compile)."""
import json
from pathlib import Path
root=Path(__file__).resolve().parent.parent
m=json.loads((root/'Production/Blueprints/blueprint_manifest_v50.json').read_text(encoding='utf-8'))
items=m['blueprints']
assert len(items)==24, len(items)
assert len({x['asset_path'] for x in items})==len(items)
for x in items:
    assert x['asset_path'].startswith('/Game/Askir/Blueprints/')
    assert x['native_path'].startswith('/Script/Askir.')
    assert x['name'].startswith('BP_')
    assert (root/'Source/Askir').exists()
print('PASS: 24 unique Blueprint definitions; source tree exists; Unreal execution still required')
