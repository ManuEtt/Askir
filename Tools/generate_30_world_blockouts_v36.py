"""Run in UE 5.6 Editor Python console: exec(open(r'ABSOLUTE_PATH_TO_SCRIPT', encoding='utf-8').read()).
Generates 30 *blockout* .umap maps, not finished levels. Back up existing maps first.
"""
import json, os, random
import unreal

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MANIFEST = os.path.join(ROOT, 'Production', 'Levels', 'world_manifest_v36.json')
OUTPUT_ROOT = '/Game/Askir/WorldBlockoutsV36'
DRY_RUN = True  # Set False only after review and backup.
OVERWRITE_EXISTING = False
CREATE_FUNCTIONAL_PORTALS = True  # Requires compiled AskirWorldPortal class

with open(MANIFEST, encoding='utf-8') as f:
    entries = json.load(f)['levels']

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
editor_assets = unreal.EditorAssetLibrary
cube_mesh = unreal.load_asset('/Engine/BasicShapes/Cube.Cube')
if not cube_mesh:
    raise RuntimeError('Engine basic Cube mesh is missing')

def place_box(name, position, scale):
    actor = editor_actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*position))
    actor.set_actor_label(name)
    actor.static_mesh_component.set_static_mesh(cube_mesh)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor

def make_level(spec):
    path = OUTPUT_ROOT + '/' + spec['map_name']
    if editor_assets.does_asset_exist(path):
        if not OVERWRITE_EXISTING:
            unreal.log_warning('SKIP existing: ' + path)
            return 'skipped'
        # Existing levels are intentionally not deleted by this script.
        unreal.log_error('Cannot overwrite existing map safely: ' + path)
        return 'blocked'
    if DRY_RUN:
        unreal.log('DRY RUN: ' + path)
        return 'dry_run'
    if not unreal.EditorLevelLibrary.new_level(path):
        raise RuntimeError('Could not create map: ' + path)
    random.seed(spec['layout_seed'])
    place_box('Ground_' + spec['id'], (0,0,-70), (36,36,1))
    # Navigable open blockout; biome-specific structures are placeholders.
    for idx in range(12):
        x = random.choice([-1400,-1150,-850,850,1150,1400])
        y = random.randrange(-1400,1401,175)
        h = random.randint(2,8)
        place_box('Blockout_%02d_%s'%(idx,spec['biome']), (x,y, h*50), (1.2,1.2,h))
    player = editor_actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0,0,140))
    player.set_actor_label('PlayerStart_'+spec['id'])
    portal_class = unreal.load_class(None, '/Script/Askir.AskirWorldPortal') if CREATE_FUNCTIONAL_PORTALS else None
    if CREATE_FUNCTIONAL_PORTALS and not portal_class:
        raise RuntimeError('Compile Askir C++ module before generating functional portals')
    for idx, dest in enumerate(spec['exits']):
        # Spread exits around perimeter; matching portal exists in destination map.
        x = -1550 if idx % 2 == 0 else 1550
        y = -1100 + (idx // 2) * 450
        if portal_class:
            portal=editor_actors.spawn_actor_from_class(portal_class, unreal.Vector(x,y,160))
            portal.set_actor_label('PORTAL_TO_'+dest)
            portal.set_editor_property('destination_map', unreal.Name(dest))
        else:
            marker=place_box('EXIT_MARKER_TO_'+dest, (x,y,150), (0.5,3,3))
            marker.set_actor_tags([unreal.Name('AskirExitMarker')])
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError('Could not save map: ' + path)
    unreal.log('CREATED blockout map: ' + path)
    return 'created'

results = {}
for spec in entries:
    try:
        results[spec['map_name']] = make_level(spec)
    except Exception as exc:
        unreal.log_error('%s: %s'%(spec['map_name'],exc))
        results[spec['map_name']] = 'failed: '+str(exc)
unreal.log('Askir V36 blockout result: ' + str(results))
