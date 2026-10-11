import json,pathlib,re
root=pathlib.Path(__file__).resolve().parent.parent
items=json.loads((root/'Production/Blueprints/blueprint_manifest_v50.json').read_text())['blueprints']
assert len(items)==24, len(items)
assert len({x['asset_path'] for x in items})==len(items)
assert all(x['asset_path'].endswith('/'+x['name']) for x in items)
assert all(x['native_path'].startswith('/Script/Askir.') for x in items)
assert all(x['native_parent'] in x['native_path'] for x in items)
assert 'BP_AskirGameMode.BP_AskirGameMode_C' in (root/'Config/DefaultEngine.ini').read_text()
assert 'default_pawn_class' in (root/'Tools/integrate_blueprints_v51.py').read_text()
assert 'DRY_RUN = True' in (root/'Tools/integrate_blueprints_v51.py').read_text()
headers='\n'.join(p.read_text(errors='ignore') for p in (root/'Source/Askir').rglob('*.h'))
missing=[x['native_parent'] for x in items if not re.search(r'class\s+(?:ASKIR_API\s+)?A'+re.escape(x['native_parent'])+r'\b',headers)]
print('Blueprint definitions:',len(items),'Native parents without detected header declaration:',sorted(set(missing)))
assert not missing, missing
print('V51 offline validation OK')
