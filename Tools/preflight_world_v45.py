"""Offline cross-file preflight for Askir 30-map pipeline. Python 3.10+."""
from pathlib import Path
import json,re,sys
ROOT=Path(__file__).resolve().parents[1]
CANONICAL='/Game/Askir/WorldV42'

def validate(root=ROOT):
    root=Path(root)
    manifest=json.loads((root/'Production/Levels/world_manifest_v36.json').read_text(encoding='utf-8'))['levels']
    designs=json.loads((root/'Production/Levels/level_designs_v42.json').read_text(encoding='utf-8'))['levels']
    errors=[]
    names=[x['map_name'] for x in manifest]
    if len(names)!=30 or len(set(names))!=30: errors.append('Manifest must have 30 unique map names')
    if set(names)!={x['map_name'] for x in designs}: errors.append('Manifest/design map names differ')
    for a,b in zip(sorted(manifest,key=lambda x:x['map_name']),sorted(designs,key=lambda x:x['map_name'])):
        if set(a['exits'])!=set(b['exits']):errors.append('Exit mismatch: '+a['map_name'])
    for rel,pattern in [
      ('Tools/generate_30_world_blockouts_v36.py',r"OUTPUT_ROOT\s*=\s*['\"]([^'\"]+)"),
      ('Tools/setup_world_portals_v43.py',r"path=['\"]([^'\"]+)/['\"]\+level"),
      ('Tools/integrate_30_level_gameplay_v44.py',r"MAP_ROOT=['\"]([^'\"]+)"),
    ]:
        content=(root/rel).read_text(encoding='utf-8')
        match=re.search(pattern,content)
        if not match or match.group(1)!=CANONICAL:errors.append('Map root mismatch: '+rel)
    portal=(root/'Source/Askir/Travel/AskirWorldPortal.cpp').read_text(encoding='utf-8')
    if CANONICAL+'/%s' not in portal:errors.append('Travel portal uses wrong map root')
    cook=(root/'Config/WorldMapsToCook_V45.ini.txt').read_text(encoding='utf-8')
    found=re.findall(r'\+MapsToCook=\(FilePath="([^"]+)"\)',cook)
    expected={CANONICAL+'/'+n for n in names}
    if len(found)!=30 or set(found)!=expected:errors.append('Cook list differs from manifest')
    return errors

if __name__=='__main__':
    errors=validate()
    for err in errors:print('ERROR:',err)
    if errors:sys.exit(1)
    print('OK: 30 maps, design exits, generator, portals, gameplay and cook paths consistent')
