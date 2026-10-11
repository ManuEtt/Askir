# Run in Unreal Engine 5.6 editor after generating V42 maps.
# DRY_RUN defaults to True. Compiled AskirWorldPortal C++ class required.
import unreal,json,os,math
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
with open(os.path.join(ROOT,'Production','Levels','level_designs_v42.json'),encoding='utf-8') as f: levels=json.load(f)['levels']
DRY_RUN=True
api=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
portal=unreal.load_class(None,'/Script/Askir.AskirWorldPortal')
if not portal:raise RuntimeError('Compile AskirWorldPortal before running')
for level in levels:
 path='/Game/Askir/WorldV42/'+level['map_name']
 if not unreal.EditorAssetLibrary.does_asset_exist(path):
  unreal.log_warning('Missing map: '+path);continue
 if DRY_RUN:
  unreal.log('PREVIEW '+path+' exits='+str(level['exits']));continue
 unreal.EditorLoadingAndSavingUtils.load_map(path)
 existing={a.get_actor_label() for a in api.get_all_level_actors()}
 count=0
 for i,destination in enumerate(level['exits']):
  label='ASKIR_PORTAL_TO_'+destination
  if label in existing:continue
  angle=2*math.pi*i/max(1,len(level['exits']))
  actor=api.spawn_actor_from_class(portal,unreal.Vector(1700*math.cos(angle),1700*math.sin(angle),180))
  if not actor:raise RuntimeError('Cannot spawn '+label)
  actor.set_actor_label(label)
  actor.set_editor_property('destination_map',unreal.Name(destination))
  count+=1
 if count and not unreal.EditorLevelLibrary.save_current_level():raise RuntimeError('Save failed: '+path)
 unreal.log('Askir '+path+' portals created: '+str(count))
