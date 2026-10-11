#!/usr/bin/env python3
"""Askir source/artwork package checks. No UE compiler required."""
from pathlib import Path
import json,sys,re
ROOT=Path(__file__).resolve().parents[1]
errors=[]
def check(cond,message):
 if not cond:errors.append(message)
check((ROOT/'AskirRPG.uproject').is_file(),'Missing .uproject')
check((ROOT/'Source/Askir/Askir.Build.cs').is_file(),'Missing module build rules')
for category in ['AI','Animation','Characters','Combat','Economy','Inventory','NPC','Save','World']:
 check((ROOT/'Source/Askir'/category).is_dir(),f'Missing C++ module folder: {category}')
for header in (ROOT/'Source/Askir').rglob('*.h'):
 s=header.read_text(encoding='utf8')
 if 'UCLASS(' in s or 'USTRUCT(' in s or 'UENUM(' in s:
  check(f'{header.stem}.generated.h' in s,f'Missing generated header: {header}')
for source in (ROOT/'Source/Askir').rglob('*.cpp'):
 check(source.stat().st_size>0,f'Empty C++ source: {source}')
assets=ROOT/'ArtSource/Enhanced3D_V19'
manifest=assets/'asset_manifest.json'
check(manifest.is_file(),'Missing enhanced 3D manifest')
if manifest.exists():
 try:
  data=json.loads(manifest.read_text(encoding='utf8'))
  check(isinstance(data,(list,dict)),'Invalid manifest type')
 except (ValueError,UnicodeError) as exc:errors.append(f'Invalid JSON: {exc}')
 glbs=list(assets.glob('SM_*.glb'))
 check(len(glbs)==13,f'Expected 13 enhanced GLBs, found {len(glbs)}')
 for glb in glbs:
  check(glb.stat().st_size>100,f'Empty GLB: {glb.name}')
  check((assets/(glb.stem+'.obj')).exists(),f'Missing OBJ for {glb.stem}')
for ignored in ['Binaries','Intermediate','Saved','DerivedDataCache']:
 check(not (ROOT/ignored).exists(),f'Generated directory should not be in release: {ignored}')
for required in ['.gitignore','.gitattributes','GITHUB_IMPORT.md','RELEASE_V20_50_SCHRITTE.md','.github/workflows/static-checks.yml']:
 check((ROOT/required).is_file(),f'Missing release file: {required}')
if errors:
 print('FAIL:',len(errors),'issues')
 for e in errors:print(' -',e)
 sys.exit(1)
print('PASS: Askir package structure, source and 3D asset checks')
