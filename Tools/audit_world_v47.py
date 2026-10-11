"""Askir V47: strict cross-file world integrity audit; standard Python, no Unreal required."""
import argparse,json,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def audit(root=ROOT):
 root=Path(root); base=root/'Production'/'Levels'
 levels=json.loads((base/'level_designs_v42.json').read_text(encoding='utf-8'))['levels']
 manifest=json.loads((base/'world_manifest_v36.json').read_text(encoding='utf-8'))['levels']
 anchors=json.loads((base/'world_gameplay_anchors_v46.json').read_text(encoding='utf-8'))['anchors']
 issues=[]; names=[x['map_name'] for x in levels]; byname={x['map_name']:x for x in levels}
 if len(names)!=30 or len(set(names))!=30:issues.append('Expected 30 unique level maps')
 if {x['map_name'] for x in manifest}!=set(names):issues.append('Manifest/level map mismatch')
 for item in manifest:
  other=byname.get(item['map_name'])
  if other and (set(item['exits'])!=set(other['exits']) or item['story_mission']!=other['story_mission']):issues.append('Manifest mismatch: '+item['map_name'])
 expected={};encounters=0;sites=0
 for lv in levels:
  name=lv['map_name']; dims=lv['dimensions_cm']
  for dest in lv['exits']:
   if dest not in byname:issues.append('Missing destination '+dest)
   elif name not in byname[dest]['exits']:issues.append('Nonreciprocal exit '+name+' -> '+dest)
  for site in lv['sites']:
   sites+=1;expected[(name,site['id'])]=('quest_anchor' if site['mission'] else 'poi_anchor',site['position'],site['mission'])
  for encounter in lv['encounters']:
   encounters+=1;expected[(name,encounter['id'])]=('encounter_anchor',encounter['position'],None)
  for point in [s['position'] for s in lv['sites']]+[e['position'] for e in lv['encounters']]:
   if len(point)!=3 or not all(isinstance(v,(int,float)) and math.isfinite(v) for v in point):issues.append('Invalid position in '+name)
   elif abs(point[0])>dims[0]/2 or abs(point[1])>dims[1]/2:issues.append('Position outside bounds '+name)
 actual={}; duplicates=[]
 for a in anchors:
  key=(a['map_name'],a['site_id'])
  if key in actual:duplicates.append(key)
  actual[key]=a
 if duplicates:issues.append('Duplicate anchor IDs: '+str(duplicates[:5]))
 for key,(kind,pos,mission) in expected.items():
  a=actual.get(key)
  if not a:issues.append('Missing anchor '+str(key));continue
  if a['kind']!=kind or a['position_cm']!=pos or a.get('mission_id')!=mission:issues.append('Anchor mismatch '+str(key))
 for key in actual.keys()-expected.keys():issues.append('Unexpected anchor '+str(key))
 if sites!=150 or encounters!=90 or len(anchors)!=240:issues.append('Expected 150 sites, 90 encounters, 240 anchors')
 seen=set();stack=[names[0]] if names else []
 while stack:
  cur=stack.pop()
  if cur in seen:continue
  seen.add(cur);stack.extend(dest for dest in byname[cur]['exits'] if dest in byname)
 if seen!=set(names):issues.append('Unreachable maps: '+str(sorted(set(names)-seen)))
 return {'ok':not issues,'maps':len(names),'sites':sites,'encounters':encounters,'anchors':len(anchors),'reachable':len(seen),'issues':issues}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--output');args=p.parse_args();result=audit()
 output=json.dumps(result,ensure_ascii=False,indent=2)
 if args.output:Path(args.output).write_text(output+'\n',encoding='utf-8')
 print(output)
 raise SystemExit(0 if result['ok'] else 1)
