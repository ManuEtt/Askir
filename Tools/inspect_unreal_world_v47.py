"""Askir V47: read-only Unreal Editor inspection of all 30 generated levels.
Never creates, changes, or saves assets. Run inside UE5.6 Python console.
"""
import json,os,collections
import unreal
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA=json.load(open(os.path.join(ROOT,'Production','Levels','level_designs_v42.json'),encoding='utf-8'))['levels']
MAP_ROOT='/Game/Askir/WorldV42'
asset=unreal.EditorAssetLibrary
actor_system=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
results=[]
for spec in DATA:
 path=MAP_ROOT+'/'+spec['map_name']
 row={'map':spec['map_name'],'exists':bool(asset.does_asset_exist(path)),'loaded':False,'anchors':0,'encounter_zones':0,'portals':0,'player_starts':0,'errors':[]}
 if not row['exists']:
  row['errors'].append('Map not generated');results.append(row);continue
 try:
  if not unreal.EditorLoadingAndSavingUtils.load_map(path):
   row['errors'].append('Map load failed');results.append(row);continue
  row['loaded']=True
  actors=actor_system.get_all_level_actors()
  labels=[a.get_actor_label() for a in actors]
  row['anchors']=sum(x.startswith('V46_') for x in labels)
  row['encounter_zones']=sum(x.startswith('V44_Encounter_') for x in labels)
  row['portals']=sum('Portal' in x for x in labels)
  row['player_starts']=sum(a.get_class().get_name()=='PlayerStart' for a in actors)
  expected=len(spec['sites'])+len(spec['encounters'])
  if row['anchors']<expected:row['errors'].append('Anchor count below expected '+str(expected))
  if row['player_starts']<1:row['errors'].append('No PlayerStart')
 except Exception as exc:row['errors'].append(str(exc))
 results.append(row)
report={'maps':len(results),'existing':sum(x['exists'] for x in results),'loaded':sum(x['loaded'] for x in results),'with_errors':sum(bool(x['errors']) for x in results),'results':results}
unreal.log('ASKIR V47 READ ONLY REPORT: '+json.dumps(report,ensure_ascii=False))
# Intentional: no save_current_level; no actor spawning; no file writes.
