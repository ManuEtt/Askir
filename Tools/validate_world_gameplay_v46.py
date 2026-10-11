"""Validate 30-region gameplay anchors without requiring Unreal Engine."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def validate(root=ROOT):
 root=Path(root)
 levels=json.loads((root/'Production/Levels/level_designs_v42.json').read_text(encoding='utf-8'))['levels']
 anchors=json.loads((root/'Production/Levels/world_gameplay_anchors_v46.json').read_text(encoding='utf-8'))['anchors']
 errors=[]
 if len(levels)!=30: errors.append('Expected 30 maps')
 if len(anchors)!=240: errors.append('Expected 240 anchors (150 POIs, 90 encounters)')
 ids=set(); maps={x['map_name'] for x in levels}; counts={m:0 for m in maps}
 for a in anchors:
  key=(a['map_name'],a['site_id'])
  if key in ids: errors.append('Duplicate anchor '+str(key))
  ids.add(key)
  if a['map_name'] not in maps: errors.append('Unknown map '+a['map_name'])
  else: counts[a['map_name']]+=1
  if a['kind'] not in ('quest_anchor','poi_anchor','encounter_anchor'): errors.append('Unknown kind '+str(key))
  if not isinstance(a['position_cm'],list) or len(a['position_cm'])!=3 or any(not isinstance(n,(int,float)) or abs(n)>100000 for n in a['position_cm']): errors.append('Invalid coordinates '+str(key))
  if a['kind']=='quest_anchor' and not a['mission_id']: errors.append('Quest anchor missing mission '+str(key))
  if a['quest_objective_id'] is not None and not a['mission_id']: errors.append('Objective without mission '+str(key))
 for m,c in counts.items():
  if c!=8: errors.append('Expected 8 anchors for '+m)
 expected={(lv['map_name'],s['id']) for lv in levels for s in lv['sites']}|{(lv['map_name'],e['id']) for lv in levels for e in lv['encounters']}
 if ids!=expected: errors.append('Anchors differ from authored level sites and encounters')
 return errors
if __name__=='__main__':
 errors=validate()
 if errors:
  for e in errors:print('ERROR:',e)
  raise SystemExit(1)
 print('OK: 30 maps, 150 POIs, 90 encounters, unique IDs and positions')
