"""Unreal Editor Python: create visible, non-functional anchor actors. Default preview mode.
Run inside Unreal Editor after V42 map generation. Never overwrite existing actors.
These anchors are NOT NPCs, quests, enemies, or finished levels.
"""
import json,os
import unreal
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ANCHORS=json.load(open(os.path.join(ROOT,'Production','Levels','world_gameplay_anchors_v46.json'),encoding='utf-8'))['anchors']
MAP_ROOT='/Game/Askir/WorldV42'
DRY_RUN=True
BY_MAP={}
for a in ANCHORS:BY_MAP.setdefault(a['map_name'],[]).append(a)
asset=unreal.EditorAssetLibrary
editor=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
results={}
for map_name,entries in BY_MAP.items():
 path=MAP_ROOT+'/'+map_name
 if not asset.does_asset_exist(path):
  results[map_name]='missing map';continue
 if DRY_RUN:
  results[map_name]='preview '+str(len(entries));continue
 if not unreal.EditorLoadingAndSavingUtils.load_map(path):
  results[map_name]='load failed';continue
 known={a.get_actor_label() for a in editor.get_all_level_actors()}
 added=0
 for a in entries:
  label='V46_'+a['kind']+'_'+a['site_id']
  if label in known:continue
  x,y,z=a['position_cm']
  actor=editor.spawn_actor_from_class(unreal.TargetPoint,unreal.Vector(x,y,z+120))
  if not actor:raise RuntimeError('Unable to spawn '+label)
  actor.set_actor_label(label);known.add(label);added+=1
 if added and not unreal.EditorLevelLibrary.save_current_level():raise RuntimeError('Unable to save '+path)
 results[map_name]='added '+str(added)
unreal.log('Askir V46 anchor results: '+str(results))
