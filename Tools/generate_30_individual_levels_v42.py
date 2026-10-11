"""Askir V42: Run inside Unreal Engine 5.6 Editor Python console.
Creates 30 individualized blockout maps. NOT production-ready levels.
BACK UP PROJECT. Change DRY_RUN to False to create maps.
"""
import json,os,random,math
import unreal
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA=os.path.join(ROOT,'Production','Levels','level_designs_v42.json')
OUTPUT='/Game/Askir/WorldV42'
DRY_RUN=True
with open(DATA,encoding='utf-8') as f: LEVELS=json.load(f)['levels']
assets=unreal.EditorAssetLibrary
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube=unreal.load_asset('/Engine/BasicShapes/Cube.Cube')
if not cube:raise RuntimeError('Engine cube mesh unavailable')
def box(label,x,y,z,sx,sy,sz):
 a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,z))
 a.set_actor_label(label)
 a.static_mesh_component.set_static_mesh(cube)
 a.set_actor_scale3d(unreal.Vector(sx,sy,sz))
 return a
def build(spec):
 path=OUTPUT+'/'+spec['map_name']
 if assets.does_asset_exist(path):
  unreal.log_warning('SKIP existing: '+path);return 'exists'
 if DRY_RUN:
  unreal.log('PREVIEW '+path+' sites='+str(len(spec['sites'])));return 'dry_run'
 if not unreal.EditorLevelLibrary.new_level(path):raise RuntimeError('Cannot create '+path)
 rng=random.Random(spec['seed']);biome=spec['biome']
 box('Walkable_Ground',0,0,-65,42,42,1)
 # A main route remains open between POIs; obstacles occupy side lanes.
 for n in range(28):
  side=-1 if n%2==0 else 1
  x=side*rng.randint(1200,1850);y=rng.randint(-1850,1850)
  height=rng.randint(2,7) if biome in ('city','fortress','ruins') else rng.randint(1,5)
  box('Environment_%02d_%s'%(n,biome),x,y,height*50,1.4,1.4,height)
 # Five unique named locations and gameplay marker anchors per map.
 for j,site in enumerate(spec['sites']):
  x,y,_=site['position']
  box('SITE_%s_%s'%(site['id'],site['name']),x,y,35,2.5,2.5,0.7)
  box('POI_%s_%s'%(site['role'],site['id']),x,y,135,0.45,0.45,1.4)
 for enc in spec['encounters']:
  x,y,_=enc['position'];box('ENCOUNTER_MARKER_'+enc['id'],x,y,80,0.65,0.65,1.6)
 player=actors.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(0,-1650,150))
 player.set_actor_label('PlayerStart_'+spec['id'])
 # Exit markers deliberately not functional until travel subsystem is validated in editor.
 for j,dest in enumerate(spec['exits']):
  angle=2*math.pi*j/max(1,len(spec['exits']))
  x=int(math.cos(angle)*1850);y=int(math.sin(angle)*1850)
  box('EXIT_TO_'+dest,x,y,160,0.5,2,3)
 if not unreal.EditorLevelLibrary.save_current_level():raise RuntimeError('Save failed '+path)
 return 'created'
results={}
for spec in LEVELS:
 try:results[spec['map_name']]=build(spec)
 except Exception as e:
  results[spec['map_name']]='ERROR: '+str(e);unreal.log_error(str(e))
unreal.log('ASKIR V42 RESULTS: '+str(results))
