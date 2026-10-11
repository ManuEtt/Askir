"""Validate encounter profiles against authored level source. No Unreal required."""
import json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def validate(levels,profiles):
 errors=[]; by_map={p['map_name']:p for p in profiles}
 if len(by_map)!=len(profiles):errors.append('duplicate map profiles')
 if len(profiles)!=len(levels):errors.append('profile count mismatch')
 ids=set()
 for level in levels:
  p=by_map.get(level['map_name'])
  if p is None:errors.append('missing profile '+level['map_name']);continue
  expected={e['id']:e for e in level['encounters']}
  found={e['encounter_id']:e for e in p['encounters']}
  if len(found)!=len(p['encounters']):errors.append('duplicate encounters '+level['map_name'])
  if expected.keys()!=found.keys():errors.append('encounter ids mismatch '+level['map_name'])
  for key,e in found.items():
   if key in ids:errors.append('global duplicate '+key)
   ids.add(key)
   if key not in expected:continue
   if e['position_cm']!=expected[key]['position']:errors.append('position mismatch '+key)
   if not isinstance(e['enemy_count'],int) or isinstance(e['enemy_count'],bool) or not 1<=e['enemy_count']<=12:errors.append('bad enemy count '+key)
   if not isinstance(e['activation_radius_cm'],(int,float)) or e['activation_radius_cm']<=0:errors.append('bad radius '+key)
   if not e['enemy_archetype']:errors.append('missing archetype '+key)
   if e['tier'] not in ('standard','elite'):errors.append('bad tier '+key)
  if p['biome']!=level['biome']:errors.append('biome mismatch '+level['map_name'])
 return errors
if __name__=='__main__':
 levels=json.loads((ROOT/'Production/Levels/level_designs_v42.json').read_text(encoding='utf8'))['levels']
 profiles=json.loads((ROOT/'Production/Levels/encounter_profiles_v48.json').read_text(encoding='utf8'))['profiles']
 errors=validate(levels,profiles)
 print(json.dumps({'regions':len(profiles),'encounters':sum(len(p['encounters']) for p in profiles),'errors':errors},ensure_ascii=False,indent=2))
 sys.exit(bool(errors))
