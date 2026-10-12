#!/usr/bin/env python3
"""Askir source/artwork package checks. No UE compiler required."""
from pathlib import Path
import json,sys,re
from collections import defaultdict
ROOT=Path(__file__).resolve().parents[1]
errors=[]
def check(cond,message):
 if not cond:errors.append(message)
check((ROOT/'AskirRPG.uproject').is_file(),'Missing .uproject')
check((ROOT/'Source/Askir/Askir.Build.cs').is_file(),'Missing module build rules')
for category in ['AI','Animation','Characters','Combat','Economy','Inventory','NPC','Save','World']:
 check((ROOT/'Source/Askir'/category).is_dir(),f'Missing C++ module folder: {category}')
declared_types=defaultdict(list)
for header in (ROOT/'Source/Askir').rglob('*.h'):
 s=header.read_text(encoding='utf8')
 if 'UCLASS(' in s or 'USTRUCT(' in s or 'UENUM(' in s:
  check(f'{header.stem}.generated.h' in s,f'Missing generated header: {header}')
  includes=re.findall(r'#\s*include\s+[<\"]([^>\"]+)[>\"]',s)
  check(bool(includes) and includes[-1]==f'{header.stem}.generated.h',f'Generated header must be last include: {header}')
 # Reject the V76 collisions between distinct NPC/player interaction classes
 # and the two global schedule structs. Forward declarations are excluded.
 for symbol in re.findall(r'\b(?:class|struct)\s+(?:ASKIR_API\s+)?([AUF]\w+)\s*[:{]',s):
  declared_types[symbol].append(str(header.relative_to(ROOT)))
 for line_number,line in enumerate(s.splitlines(),1):
  if re.search(r'UPROPERTY\([^;]+\)\s*(?:float|int32|FText|FName|bool)\s+[^;]+,[^;]+;',line):
   errors.append(f'Multiple declarations in one reflected property: {header}:{line_number}')
for symbol,headers in declared_types.items():
 check(len(headers)==1,f'Duplicate type definition {symbol}: {headers}')
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
print('PASS: offline Askir package, reflection structure and raw-asset checks (Unreal not compiled)')
