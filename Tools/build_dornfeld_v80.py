"""UE5.6 two-map editor pipeline for Dornfeld and its canonical adjoining countryside.

Offline preflight validates source contracts only. Actual packages are created,
reloaded and checked exclusively in UE Python. Startup changes occur once both
maps, reciprocal routes, objective references and package files have validated.
"""
from __future__ import annotations
import argparse
import importlib.util
import json
import math
import os
from pathlib import Path
import traceback

ROOT = Path(__file__).resolve().parents[1]
MAP_PATH = '/Game/Askir/Maps/Dornfeld_V80'
OUTSKIRTS_PATH = '/Game/Askir/Maps/Dornfelder_Umland_V80'
MAP_PATHS = (MAP_PATH, OUTSKIRTS_PATH)
CONTRACT_PATH = 'Production/Levels/dornfeld_outskirts_v80.json'
PROP_PACKAGES = {'SM_AskirDeliveryCart': '/Game/Askir/V80/Props/SM_AskirDeliveryCart',
                 'SM_AskirSupplyCrate': '/Game/Askir/V80/Props/SM_AskirSupplyCrate'}
CART_POSITION = (-350,2500,0)


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec);spec.loader.exec_module(result)
    return result


def finite_vector(value, label):
    if not isinstance(value, list) or len(value) != 3 or any(isinstance(x, bool) or not isinstance(x, (int, float)) or not math.isfinite(x) for x in value):
        raise ValueError('Invalid finite 3D coordinates: ' + label)
    return value


def validate_portal_transform(rotation, scale):
    """World arrival sums are valid only for the authored local-space transform."""
    finite_vector(rotation,'portal rotation');finite_vector(scale,'portal scale')
    if any(abs(value)>.01 for value in rotation): raise RuntimeError('Arrival geometry requires the authored zero portal rotation')
    if any(abs(value-1)>.001 for value in scale): raise RuntimeError('Arrival geometry requires the authored positive unit portal scale')


def validate_contract(data, reward_item, quest):
    if data.get('version') != 'V80' or data.get('chapter') != 1 or data.get('continent') != 'Ardem':
        raise ValueError('V80 chapter-one Ardem contract required')
    if data.get('quest_id') != 'SQ_001' or data.get('reward_item_id') != 'item_002' or data.get('reward_asset') != '/Game/Askir/V80/Items/item_002':
        raise ValueError('Canonical lost-delivery quest/reward identity changed')
    if quest.get('giver') != 'Torben' or quest.get('rewards') != {'xp': 100, 'gold': 16, 'item': 'item_002'}:
        raise ValueError('Canonical SQ_001 giver/rewards changed; reconcile native mission first')
    if {key: reward_item.get(key) for key in ('id', 'category', 'value', 'stack_max', 'weight')} != {'id': 'item_002', 'category': 'ingredient', 'value': 17, 'stack_max': 99, 'weight': .6}:
        raise ValueError('Canonical ingredient reward changed; reconcile mission first')
    maps = data.get('maps', [])
    if len(maps) != 2 or tuple(entry.get('package') for entry in maps) != MAP_PATHS:
        raise ValueError('Both exact V80 map packages are required in route order')
    if maps[0].get('ground_center') != [0,500,-50] or maps[0].get('ground_extent') != [2400,3200,50] or maps[0].get('start') != [0,-1700,100]:
        raise ValueError('Village geometry must match the preserved V77-V79 well-mission generator')
    if maps[1].get('ground_center') != [0,1600,-50] or maps[1].get('ground_extent') != [3200,4500,50] or maps[1].get('start') != [0,-1780,100]:
        raise ValueError('Countryside geometry must contain the authored farms, camp, props and woodland')
    portal_ids = [entry.get('portal', {}).get('id') for entry in maps]
    if portal_ids != ['DORNFELD_TO_UMLAND_V80', 'UMLAND_TO_DORNFELD_V80']:
        raise ValueError('Both stable reciprocal route IDs are required')
    for index, entry in enumerate(maps):
        portal = entry['portal'];other = maps[1-index]
        if not entry.get('name') or not entry.get('function') or not portal.get('interaction_label'):
            raise ValueError('Map/route narrative labels cannot be empty')
        if portal.get('destination_id') != other['portal']['id'] or portal.get('destination_package') != other['package']:
            raise ValueError('Route destination must resolve to the opposite map and portal')
        for field in ('start', 'ground_center', 'ground_extent'): finite_vector(entry[field], field)
        for field in ('position', 'arrival_offset', 'trigger_extent'): finite_vector(portal[field], field)
        if any(value <= 0 for value in entry['ground_extent'] + portal['trigger_extent']): raise ValueError('Ground and trigger extents must be positive')
        # Authoring uses zero rotation: local arrival offsets are in map coordinates.
        arrival = [x+y for x,y in zip(portal['position'], portal['arrival_offset'])]
        extent = portal['trigger_extent']
        if all(abs(portal['arrival_offset'][axis]) <= extent[axis] + (42 if axis < 2 else 96) for axis in range(3)):
            raise ValueError('Arrival capsule must be wholly outside the route trigger')
        for label, point in (('start', entry['start']), ('arrival', arrival), ('portal', portal['position'])):
            validate_on_ground(point, entry, label)
    if data.get('giver', {}).get('event_id') != 'SQ_001_TORBEN' or data['giver'].get('label') != 'Torben': raise ValueError('Torben giver identity changed')
    finite_vector(data['giver']['position'], 'giver');validate_on_ground(data['giver']['position'], maps[0], 'giver')
    bandits = data.get('bandits', [])
    if len(bandits) != 2 or [entry.get('id') for entry in bandits] != ['SQ_001_BANDIT_1', 'SQ_001_BANDIT_2']:
        raise ValueError('Exactly two distinct canonical bandit receipts are required')
    for entry in bandits:
        finite_vector(entry['position'], entry['id']);validate_on_ground(entry['position'], maps[1], entry['id'])
    if data.get('cargo', {}).get('event_id') != 'SQ_001_CARGO' or data['cargo'].get('label') != 'Torbens_Lieferung': raise ValueError('Cargo receipt identity changed')
    finite_vector(data['cargo']['position'], 'cargo');validate_on_ground(data['cargo']['position'], maps[1], 'cargo')
    if len({tuple(entry['position']) for entry in bandits} | {tuple(data['cargo']['position'])}) != 3: raise ValueError('Bandit and cargo placements must be distinct')
    return data


def validate_on_ground(point, map_spec, label):
    center, extent = map_spec['ground_center'], map_spec['ground_extent']
    if any(abs(point[axis]-center[axis]) > extent[axis]-100 for axis in (0, 1)):
        raise ValueError('Objective/start outside safe authored ground: ' + label)
    ground_z = center[2]+extent[2]
    if not 40 <= point[2]-ground_z <= 140: raise ValueError('Start/objective height outside ground-trace range: ' + label)


def source_preflight(root=ROOT, contract_path=None):
    root = Path(root)
    data = json.loads(Path(contract_path or root / CONTRACT_PATH).read_text(encoding='utf-8'))
    items = json.loads((root / 'Production/Items/items.json').read_text(encoding='utf-8'))
    quests = json.loads((root / 'Production/Quests/sidequests.json').read_text(encoding='utf-8'))
    rewards = [entry for entry in items if entry.get('id') == data.get('reward_item_id')]
    missions = [entry for entry in quests if entry.get('id') == data.get('quest_id')]
    if len(rewards) != 1 or len(missions) != 1: raise ValueError('Canonical quest and reward must resolve uniquely')
    validate_contract(data, rewards[0], missions[0])
    module('_v80_economy_preflight', root / 'Tools/build_dornfeld_v79.py').source_preflight(root)
    module('_v80_character_preflight', root / 'Tools/import_animated_assets_v78.py').source_preflight(root)
    module('_v80_prop_preflight', root / 'Tools/import_delivery_props_v80.py').source_preflight(root)
    return data


def saved_map(root, package):
    path = Path(root) / ('Content/' + package.removeprefix('/Game/') + '.umap')
    if not path.is_file() or path.stat().st_size < 1024: raise RuntimeError('Real saved world package missing: ' + str(path))
    return str(path)


def require_world(unreal, root, package):
    saved_map(root, package)
    if not isinstance(unreal.load_asset(package), unreal.World): raise RuntimeError('Saved map package is not a UWorld: ' + package)
    if not unreal.EditorLevelLibrary.load_level(package): raise RuntimeError('Saved world cannot reload: ' + package)


def protect_existing_maps(unreal, root):
    for package in MAP_PATHS:
        physical = Path(root) / ('Content/' + package.removeprefix('/Game/') + '.umap')
        if physical.exists() or unreal.EditorAssetLibrary.does_asset_exist(package):
            raise RuntimeError('Existing V80 world preserved; use --use-existing-map to validate both maps: ' + package)


def spawn(unreal, actor_api, cls, label, position, rotation=(0,0,0)):
    if any(actor.get_actor_label() == label for actor in actor_api.get_all_level_actors()): raise RuntimeError('Duplicate actor label: ' + label)
    actor = actor_api.spawn_actor_from_class(cls, unreal.Vector(*position), unreal.Rotator(*rotation))
    if actor is None: raise RuntimeError('Actor spawn failed: ' + label)
    actor.set_actor_label(label);actor.set_editor_property('tags', [unreal.Name('ASKIR_V80')])
    return actor


def static_mesh(unreal, actor, shape, scale, material, property_name='static_mesh_component', collides=True):
    component = actor.get_editor_property(property_name)
    asset = unreal.load_asset('/Engine/BasicShapes/' + shape + '.' + shape)
    if asset is None: raise RuntimeError('Licensed engine primitive missing: ' + shape)
    component.set_static_mesh(asset);component.set_relative_scale3d(unreal.Vector(*scale));component.set_material(0, material)
    component.set_collision_profile_name('BlockAll' if collides else 'NoCollision')
    return component


def delivery_mesh(unreal, actor, package, property_name='static_mesh_component', offset=None):
    """Original OBJ meshes are authored in metres, imported once with scale 100."""
    mesh = unreal.load_asset(package)
    if not isinstance(mesh, unreal.StaticMesh): raise RuntimeError('Validated original delivery prop missing: ' + package)
    component = actor.get_editor_property(property_name)
    component.set_static_mesh(mesh);component.set_relative_scale3d(unreal.Vector(1,1,1))
    # Cargo has a separate SceneRoot; never offset a StaticMeshActor's root.
    if offset is not None: component.set_relative_location(unreal.Vector(*offset), False, False)
    component.set_collision_profile_name('BlockAll')
    return component


def block(unreal, actor_api, label, position, scale, material, shape='Cube'):
    actor = spawn(unreal, actor_api, unreal.StaticMeshActor, label, position)
    static_mesh(unreal, actor, shape, scale, material)
    return actor


def perimeter(unreal, actor_api, spec, prefix, material):
    """Keep the playable blockout inside its finite colliding ground mesh."""
    center, extent = spec['ground_center'], spec['ground_extent']
    for label,position,scale in (
        ('West',(center[0]-extent[0]+50,center[1],130),(.6,(extent[1]*2-100)/100,2.6)),
        ('East',(center[0]+extent[0]-50,center[1],130),(.6,(extent[1]*2-100)/100,2.6)),
        ('North',(center[0],center[1]+extent[1]-50,130),((extent[0]*2-100)/100,.6,2.6)),
        ('South',(center[0],center[1]-extent[1]+50,130),((extent[0]*2-100)/100,.6,2.6))):
        block(unreal,actor_api,prefix+'_Boundary_'+label,position,scale,material)


def portal(unreal, actor_api, map_spec):
    spec = map_spec['portal']
    actor = spawn(unreal, actor_api, unreal.AskirWorldPortal, spec['id'], spec['position'])
    values = {'portal_id': unreal.Name(spec['id']), 'destination_portal_id': unreal.Name(spec['destination_id']),
              'destination_map': unreal.Name(spec['destination_package'].rsplit('/', 1)[-1]), 'destination_package_path': spec['destination_package'],
              'arrival_offset': unreal.Vector(*spec['arrival_offset']), 'interaction_range': 280.0,
              'interaction_label': unreal.Text(spec['interaction_label']), 'travel_on_overlap': False, 'enabled': True}
    village = module('_v80_portal_bool', ROOT / 'Tools/build_dornfeld_v77.py')
    for field, value in values.items():
        if isinstance(value, bool): village.set_boolean_property(actor, field, 'b' + ''.join(x.title() for x in field.split('_')), value)
        else: actor.set_editor_property(field, value)
    actor.get_editor_property('trigger').set_box_extent(unreal.Vector(*spec['trigger_extent']))
    # The route is visible; static gateposts sit outside its trigger and arrival corridor.
    timber = unreal.load_asset('/Game/Askir/V77/Materials/M_DornfeldTimber')
    stone = unreal.load_asset('/Game/Askir/V77/Materials/M_DornfeldStone')
    x,y,z = spec['position']
    for side in (-1,1): block(unreal, actor_api, spec['id']+'_Gatepost_'+str(side), (x+side*300,y,140), (.7, .7, 2.8), timber)
    block(unreal, actor_api, spec['id']+'_Lintel', (x,y,310), (6.7,.7,.5), stone)
    return actor


def reward_asset(unreal, root, data, village):
    canonical = next(entry for entry in json.loads((root / 'Production/Items/items.json').read_text(encoding='utf-8')) if entry['id'] == data['reward_item_id'])
    spec = {'id': canonical['id'], 'name': canonical['name'], 'description': canonical['description'], 'type': 'Misc',
            'stack': canonical['stack_max'], 'weight': canonical['weight'], 'value': canonical['value'], 'damage': 0, 'heal': 0, 'attack_range': 150, 'attack_cooldown': .7}
    economy = module('_v80_reward_builder', root / 'Tools/build_dornfeld_v79.py')
    return economy.economy_assets(unreal, root, {'items': [spec], 'asset_root': data['reward_asset'].rsplit('/',1)[0]}, village)[canonical['id']]


def place_giver(unreal, actor_api, data):
    spec = data['giver'];actor = spawn(unreal, actor_api, unreal.AskirDeliveryQuestActor, spec['label'], spec['position'])
    actor.set_editor_property('role', unreal.AskirDeliveryQuestRole.GIVER);actor.set_editor_property('event_id', unreal.Name(spec['event_id']))
    actor.set_editor_property('display_name', unreal.Text('Torben – Karawanenhändler'));actor.set_editor_property('interaction_range', 250.)
    static_mesh(unreal, actor, 'Cylinder', (.65,.65,1.8), unreal.load_asset('/Game/Askir/V77/Materials/M_DornfeldMara'), 'visual')
    return actor


def generate_outskirts(unreal, root, data):
    spec = data['maps'][1];actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not unreal.EditorLevelLibrary.new_level(OUTSKIRTS_PATH): raise RuntimeError('Cannot create real countryside map')
    mats = {key: unreal.load_asset('/Game/Askir/V77/Materials/M_Dornfeld'+name) for key,name in (('earth','Earth'),('timber','Timber'),('stone','Stone'),('roof','Roof'))}
    if any(value is None for value in mats.values()): raise RuntimeError('Inherited environment material missing')
    block(unreal, actor_api, 'Umland_Ground', spec['ground_center'], [v*2/100 for v in spec['ground_extent']], mats['earth'])
    block(unreal, actor_api, 'Umland_TradeRoad', (0,1250,2), (4.5,79,.04), mats['stone'])
    # Two solid farmhouses and planted rows leave a broad, unobstructed quest road.
    for index,x in enumerate((-1800,1800)):
        block(unreal, actor_api, 'Umland_Farm_'+str(index), (x,-500,160), (6,7,3.2), mats['timber'])
        block(unreal, actor_api, 'Umland_FarmRoof_'+str(index), (x,-500,370), (8,9,1.5), mats['roof'], 'Cone')
        for row in range(4): block(unreal, actor_api, 'Umland_Field_%d_%d'%(index,row), (x+row*130-200,300,18), (.5,8,.36), mats['earth'])
    # Original blockout woodland, not imported production foliage. Avoid the road,
    # objective arena, portal approach and farm collision envelopes.
    for index,(x,y) in enumerate(((-2600,-1700),(2600,-1500),(-2700,1300),(2600,1400),(-2100,2300),(2100,2300),(-2600,3400),(2600,3500),(-1800,4200),(1900,4300),(-900,4900),(900,5100))):
        block(unreal, actor_api, 'Umland_TreeTrunk_'+str(index), (x,y,160), (.6,.6,3.2), mats['timber'], 'Cylinder')
        block(unreal, actor_api, 'Umland_TreeCrown_'+str(index), (x,y,430), (4.8,4.8,5.4), mats['earth'], 'Cone')
    for index,x in enumerate((-900,900)):
        block(unreal, actor_api, 'Umland_CampTent_'+str(index), (x,2300,140), (5.5,5.5,2.8), mats['roof'], 'Cone')
    block(unreal, actor_api, 'Umland_CampBack', (0,2850,110), (21,.5,2.2), mats['timber'])
    perimeter(unreal,actor_api,spec,'Umland',mats['stone'])
    spawn(unreal, actor_api, unreal.PlayerStart, 'UmlandPlayerStart', spec['start'], (0,90,0));portal(unreal, actor_api, spec)
    village = module('_v80_bandit_flags', root / 'Tools/build_dornfeld_v77.py')
    for enemy in data['bandits']:
        actor = spawn(unreal, actor_api, unreal.AskirNPC, enemy['id'], enemy['position'], (0,-90,0))
        actor.set_editor_property('npc_name', unreal.Text('Straßenbandit'));village.set_boolean_property(actor,'hostile','bHostile',True)
        for name,value in (('kill_experience',0),('defeat_objective_id',unreal.Name('None')),('detection_radius',900.),('max_chase_distance',1500.),('attack_damage',7.),('attack_interval',1.6)):
            actor.set_editor_property(name,value)
        actor.set_editor_property('guaranteed_drop',None)
        if not actor.get_editor_property('animation_playback').configure_character_visual(unreal.Name('Raider')): raise RuntimeError('Bandit rig setup failed')
        relay = spawn(unreal, actor_api, unreal.AskirDeliveryBanditRelay, 'Relay_'+enemy['id'], (0,0,0))
        relay.set_editor_property('event_id',unreal.Name(enemy['id']));relay.set_editor_property('bandit_actor',actor)
    cargo = data['cargo'];actor = spawn(unreal, actor_api, unreal.AskirDeliveryQuestActor, cargo['label'], cargo['position'])
    actor.set_editor_property('role',unreal.AskirDeliveryQuestRole.CARGO);actor.set_editor_property('event_id',unreal.Name(cargo['event_id']))
    actor.set_editor_property('display_name',unreal.Text('Torbens versiegelte Lieferung'));actor.set_editor_property('interaction_range',250.)
    delivery_mesh(unreal,actor,PROP_PACKAGES['SM_AskirSupplyCrate'],'visual',(0,0,-cargo['position'][2]))
    wagon = spawn(unreal,actor_api,unreal.StaticMeshActor,'Torbens_Lieferwagen',CART_POSITION,(0,90,0))
    delivery_mesh(unreal,wagon,PROP_PACKAGES['SM_AskirDeliveryCart'])
    sun = spawn(unreal,actor_api,unreal.DirectionalLight,'Umland_Sun',(0,0,1500),(-35,-30,0))
    sun.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sun.light_component.set_editor_property('intensity',3.)
    sky = spawn(unreal,actor_api,unreal.SkyLight,'Umland_SkyLight',(0,0,1000))
    sky.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sky.light_component.set_editor_property('intensity',.9)
    spawn(unreal,actor_api,unreal.SkyAtmosphere,'Umland_Atmosphere',(0,0,0));spawn(unreal,actor_api,unreal.ExponentialHeightFog,'Umland_Fog',(0,0,20))
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.configure_editor_game_mode(world,unreal.AskirGameMode,True): raise RuntimeError('Countryside native game mode setup failed')
    nav = unreal.AskirEditorLevelTools.create_navigation_bounds(world,unreal.Vector(0,1600,150),unreal.Vector(3100,4400,350))
    if nav is None: raise RuntimeError('Countryside navigation brush creation failed')
    nav.set_actor_label('Umland_NavigationBounds')
    if not unreal.AskirEditorLevelTools.build_editor_navigation(world): raise RuntimeError('Countryside navigation build failed')
    if not unreal.EditorLevelLibrary.save_current_level(): raise RuntimeError('Countryside world save failed')


def expected_targets(data, package):
    spec = next(entry for entry in data['maps'] if entry['package'] == package)
    return [spec['portal']['id']] + ([data['giver']['label']] if package == MAP_PATH else [entry['id'] for entry in data['bandits']] + [data['cargo']['label']])


def validate_route_map(unreal, actor_api, root, data, package):
    require_world(unreal,root,package)
    actors = actor_api.get_all_level_actors();labels = {actor.get_actor_label():actor for actor in actors}
    if len(labels) != len(actors): raise RuntimeError('Duplicate actor labels in saved map: '+package)
    spec = next(entry for entry in data['maps'] if entry['package'] == package);route = spec['portal']
    prefix = 'Dornfeld' if package == MAP_PATH else 'Umland'
    for side in ('West','East','North','South'):
        wall = labels.get(prefix+'_Boundary_'+side)
        if wall is None or str(wall.get_editor_property('static_mesh_component').get_collision_profile_name()) != 'BlockAll': raise RuntimeError('Blockout edge collision missing: '+side)
    matches = [actor for actor in actors if isinstance(actor,unreal.AskirWorldPortal)]
    if len(matches) != 1 or matches[0].get_actor_label() != route['id']: raise RuntimeError('Saved map requires exactly its one reciprocal route')
    actor = matches[0]
    for field,value in (('portal_id',route['id']),('destination_portal_id',route['destination_id']),('destination_package_path',route['destination_package']),('destination_map',route['destination_package'].rsplit('/',1)[-1])):
        if str(actor.get_editor_property(field)) != value: raise RuntimeError('Saved route identity mismatch: '+field)
    for field,expected in (('arrival_offset',route['arrival_offset']),):
        value = actor.get_editor_property(field)
        if any(abs(actual-target)>.01 for actual,target in zip((value.x,value.y,value.z),expected)): raise RuntimeError('Saved arrival offset changed')
    location = actor.get_actor_location()
    if any(abs(actual-target)>.01 for actual,target in zip((location.x,location.y,location.z),route['position'])): raise RuntimeError('Saved portal placement changed')
    rotation = actor.get_actor_rotation()
    scale = actor.get_actor_scale3d()
    validate_portal_transform([rotation.pitch,rotation.yaw,rotation.roll],[scale.x,scale.y,scale.z])
    extent = actor.get_editor_property('trigger').get_scaled_box_extent()
    if any(abs(actual-target)>.01 for actual,target in zip((extent.x,extent.y,extent.z),route['trigger_extent'])): raise RuntimeError('Saved portal trigger extent changed')
    if actor.get_editor_property('travel_on_overlap') or not actor.get_editor_property('enabled'): raise RuntimeError('V80 routes require explicit interaction and enabled state')
    if not str(actor.get_editor_property('required_quest_id')) in ('None','') or actor.get_editor_property('require_quest_completed'): raise RuntimeError('Umland exploration route unexpectedly quest-gated')
    start_label = 'DornfeldPlayerStart' if package == MAP_PATH else 'UmlandPlayerStart'
    targets = expected_targets(data,package)
    for label in [start_label]+targets:
        if label not in labels or not unreal.AskirEditorLevelTools.has_walkable_ground_below(labels[label]): raise RuntimeError('Missing/walkability-invalid objective: '+label)
    objective_specs = [data['giver']] if package == MAP_PATH else data['bandits']+[data['cargo']]
    for objective in objective_specs:
        actor = labels[objective.get('label',objective.get('id'))];point = actor.get_actor_location()
        if any(abs(actual-expected)>.01 for actual,expected in zip((point.x,point.y,point.z),objective['position'])): raise RuntimeError('Saved objective placement differs')
    if package == MAP_PATH:
        giver = labels[data['giver']['label']]
        if not isinstance(giver,unreal.AskirDeliveryQuestActor) or giver.get_editor_property('role') != unreal.AskirDeliveryQuestRole.GIVER or str(giver.get_editor_property('event_id')) != data['giver']['event_id']: raise RuntimeError('Torben quest giver binding lost')
    else:
        cargo = labels[data['cargo']['label']]
        if not isinstance(cargo,unreal.AskirDeliveryQuestActor) or cargo.get_editor_property('role') != unreal.AskirDeliveryQuestRole.CARGO or str(cargo.get_editor_property('event_id')) != data['cargo']['event_id']: raise RuntimeError('Cargo quest receipt binding lost')
        for label,package,property_name in ((data['cargo']['label'],PROP_PACKAGES['SM_AskirSupplyCrate'],'visual'),('Torbens_Lieferwagen',PROP_PACKAGES['SM_AskirDeliveryCart'],'static_mesh_component')):
            if label not in labels: raise RuntimeError('Original delivery prop actor missing: '+label)
            visual = labels[label].get_editor_property(property_name)
            mesh = visual.get_editor_property('static_mesh')
            if not isinstance(mesh,unreal.StaticMesh) or mesh.get_path_name().split('.',1)[0] != package or str(visual.get_collision_profile_name()) != 'BlockAll': raise RuntimeError('Original delivery prop mesh/collision lost: '+label)
            scale = visual.get_editor_property('relative_scale3d')
            if any(abs(value-1)>.001 for value in (scale.x,scale.y,scale.z)): raise RuntimeError('Delivery prop must retain centimetre import scale')
        cargo_offset = cargo.get_editor_property('visual').get_editor_property('relative_location')
        if any(abs(actual-expected)>.01 for actual,expected in zip((cargo_offset.x,cargo_offset.y,cargo_offset.z),(0,0,-data['cargo']['position'][2]))): raise RuntimeError('Cargo visual must rest on ground with its interaction position preserved')
        cargo_scale = cargo.get_actor_scale3d()
        if any(abs(value-1)>.001 for value in (cargo_scale.x,cargo_scale.y,cargo_scale.z)): raise RuntimeError('Cargo root scale must preserve original geometry and floor placement')
        cart_location = labels['Torbens_Lieferwagen'].get_actor_location()
        if any(abs(actual-expected)>.01 for actual,expected in zip((cart_location.x,cart_location.y,cart_location.z),CART_POSITION)): raise RuntimeError('Delivery cart placement changed')
        cart_rotation = labels['Torbens_Lieferwagen'].get_actor_rotation()
        if any(abs(actual-expected)>.01 for actual,expected in zip((cart_rotation.pitch,cart_rotation.yaw,cart_rotation.roll),(0,90,0))): raise RuntimeError('Delivery cart road alignment changed')
        for enemy in data['bandits']:
            bandit = labels[enemy['id']];relay = labels.get('Relay_'+enemy['id'])
            if not isinstance(bandit,unreal.AskirNPC) or not isinstance(relay,unreal.AskirDeliveryBanditRelay): raise RuntimeError('Native bandit/relay lost')
            if relay.get_editor_property('bandit_actor') != bandit or str(relay.get_editor_property('event_id')) != enemy['id']: raise RuntimeError('Bandit receipt/reference mismatch')
            if bandit.get_editor_property('kill_experience') != 0 or str(bandit.get_editor_property('defeat_objective_id')) not in ('None','') or bandit.get_editor_property('guaranteed_drop') is not None: raise RuntimeError('Bandit can award duplicate generic reward')
            if not bandit.get_editor_property('hostile'): raise RuntimeError('Delivery bandit is not hostile')
        if 'Umland_Ground' not in labels or 'Umland_NavigationBounds' not in labels: raise RuntimeError('Saved countryside floor/navigation brush missing')
        floor = labels['Umland_Ground'].get_editor_property('static_mesh_component')
        if floor.get_editor_property('static_mesh') is None or str(floor.get_collision_profile_name()) != 'BlockAll': raise RuntimeError('Countryside floor mesh/collision missing')
        extent = labels['Umland_NavigationBounds'].get_actor_bounds(False)[1]
        if extent.x < 3000 or extent.y < 4300 or extent.z < 300: raise RuntimeError('Countryside navigation brush geometry lost')
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not unreal.AskirEditorLevelTools.configure_editor_game_mode(world,unreal.AskirGameMode,False): raise RuntimeError('Saved native game mode lost')
    arrival = [x+y for x,y in zip(route['position'],route['arrival_offset'])]
    capsule = unreal.AskirEditorLevelTools.get_player_capsule_extent()
    capsule_extent = [capsule.x,capsule.y,capsule.z]
    if any(not math.isfinite(value) or value <= 0 for value in capsule_extent): raise RuntimeError('Native player capsule invalid')
    if all(abs(route['arrival_offset'][axis]) <= route['trigger_extent'][axis]+capsule_extent[axis] for axis in range(3)): raise RuntimeError('Actual native arrival capsule intersects portal trigger')
    if not unreal.AskirEditorLevelTools.is_player_arrival_clear(world,unreal.Vector(*arrival)): raise RuntimeError('Arrival capsule overlaps blocking collision or lacks walkable floor')
    if not unreal.AskirEditorLevelTools.is_player_arrival_clear(world,labels[start_label].get_actor_location()): raise RuntimeError('PlayerStart capsule overlaps blocking collision or lacks walkable floor')
    economy = module('_v80_approach',root/'Tools/build_dornfeld_v79.py')
    start = labels[start_label].get_actor_location()
    for label in targets:
        endpoint = economy.approach_point(unreal,start,labels[label].get_actor_location(),190.)
        path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world,start,endpoint)
        if path is None or not path.is_valid() or path.is_partial(): raise RuntimeError('No complete start-to-objective navigation path: '+label)
    path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world,unreal.Vector(*arrival),start)
    if path is None or not path.is_valid() or path.is_partial(): raise RuntimeError('Arrival has no complete path to map start')
    village = module('_v80_route_packages',root/'Tools/build_dornfeld_v77.py')
    village.assert_saved_package(root,data['reward_asset'])
    return {'status':'Bestanden','map_reload':True,'world_asset_type':'UWorld','map_file':saved_map(root,package),
            'native_game_mode':'Bestanden','objective_references':'Bestanden','spawn_collision_traces':'Bestanden',
            'navigation_paths':{'status':'Bestanden','targets':targets},'arrival_collision':'Bestanden',
            'player_capsule_extent':capsule_extent,
            'route':route,'asset_package_files':[data['reward_asset']],
            'coverage':'saved editor references, real UWorld reload, ground/capsule collision and navigation; no runtime travel/playthrough'}


def configure_startup(root, village, results):
    if set(results) != set(MAP_PATHS) or any(result.get('status') != 'Bestanden' or result.get('map_reload') is not True or result.get('world_asset_type') != 'UWorld' or any(result.get(key) != 'Bestanden' for key in ('arrival_collision','objective_references','native_game_mode','spawn_collision_traces','delivery_props')) or result.get('navigation_paths',{}).get('status') != 'Bestanden' for result in results.values()):
        raise RuntimeError('Startup requires both saved maps and all route/objective/collision/navigation validations')
    for package in MAP_PATHS: saved_map(root,package)
    village.MAP_PATH = MAP_PATH
    result = village.configure_startup_maps(root,results[MAP_PATH])
    result['maps_to_cook'] = list(MAP_PATHS)
    return result


def generate(unreal, root=ROOT, validate_only=False):
    root = Path(root);data = source_preflight(root)
    if not unreal.SystemLibrary.get_engine_version().startswith('5.6.'): raise RuntimeError('UE5.6 required')
    for cls in ('AskirDeliveryQuestActor','AskirDeliveryQuestRole','AskirDeliveryBanditRelay','AskirWorldPortal','AskirEditorLevelTools'):
        if getattr(unreal,cls,None) is None: raise RuntimeError('Compile AskirEditor first: '+cls)
    village = module('_v80_village',root/'Tools/build_dornfeld_v77.py');village.MAP_PATH = MAP_PATH
    base = module('_v80_preserved_v79',root/'Tools/build_dornfeld_v79.py');base.MAP_PATH = MAP_PATH
    base.configure_startup = lambda *args: {'status':'Nicht ausgeführt','reason':'waiting for both V80 worlds and reciprocal routes'}
    actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not validate_only:
        protect_existing_maps(unreal,root)
        prop_importer = module('_v80_delivery_prop_import',root/'Tools/import_delivery_props_v80.py')
        imported_props = prop_importer.import_delivery_props(unreal,root)
        if imported_props.get('status') != 'Bestanden' or imported_props.get('assets') != PROP_PACKAGES: raise RuntimeError('Original prop import contract incomplete')
        base.generate(unreal,root)
        reward_asset(unreal,root,data,village);place_giver(unreal,actor_api,data);portal(unreal,actor_api,data['maps'][0])
        perimeter(unreal,actor_api,data['maps'][0],'Dornfeld',unreal.load_asset('/Game/Askir/V77/Materials/M_DornfeldStone'))
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        if not unreal.AskirEditorLevelTools.build_editor_navigation(world) or not unreal.EditorLevelLibrary.save_current_level(): raise RuntimeError('Village route navigation/save failed')
        generate_outskirts(unreal,root,data)
    results = {}
    for package in MAP_PATHS:
        results[package] = validate_route_map(unreal,actor_api,root,data,package)
        if package == MAP_PATH:
            inherited = base.generate(unreal,root,validate_only=True)['validation']
            results[package].update(inherited)
            # Preserve route validation after inherited fields are merged.
            results[package]['navigation_paths'] = {'status':'Bestanden','targets':expected_targets(data,package),'mission_targets':inherited['navigation_paths']['targets']}
            results[package]['asset_package_files'] = list(dict.fromkeys(inherited['asset_package_files']+[data['reward_asset']]))
    # Validate rig packages on validation-only executions, too. No reimport/mutation.
    importer = module('_v80_validate_characters',root/'Tools/import_animated_assets_v78.py')
    # Importer source_preflight already verifies all sources. Rig packages must load
    # and be materialized; every supported role/clip is a cook dependency.
    animation_packages = [];skeleton_packages = []
    for role in ('Hero','Guard','Raider'):
        mesh = unreal.load_asset('/Game/Askir/CharactersV78/'+role+'/SK_Askir'+role)
        if not isinstance(mesh,unreal.SkeletalMesh): raise RuntimeError('Native character mesh unavailable: '+role)
        skeleton = mesh.get_editor_property('skeleton')
        if not isinstance(skeleton,unreal.Skeleton): raise RuntimeError('Native character skeleton unavailable: '+role)
        skeleton_path = skeleton.get_path_name().split('.',1)[0]
        village.assert_saved_package(root,skeleton_path);skeleton_packages.append(skeleton_path)
        for leaf,cls in [('SK_Askir'+role,unreal.SkeletalMesh)] + [('A_'+clip,unreal.AnimSequence) for clip in ('Idle','Walk','Run','Sprint','Jump','Dodge','LightAttack','HeavyAttack','Hit','Death','Block','Cast')]:
            package = '/Game/Askir/CharactersV78/'+role+'/'+leaf
            asset = unreal.load_asset(package)
            if not isinstance(asset,cls): raise RuntimeError('Required character package missing/wrong type: '+package)
            if cls == unreal.AnimSequence: importer.validate_sequence(asset,skeleton,package)
            village.assert_saved_package(root,package);animation_packages.append(package)
    prop_importer = module('_v80_delivery_prop_validation',root/'Tools/import_delivery_props_v80.py')
    prop_validation = prop_importer.validate_imported_props(unreal,root)
    if prop_validation.get('status') != 'Bestanden' or prop_validation.get('assets') != PROP_PACKAGES: raise RuntimeError('Original prop validation contract incomplete')
    for result in results.values():
        result['asset_package_files'] = list(dict.fromkeys(result['asset_package_files']+animation_packages+skeleton_packages))
        result['animation_sequences'] = 'Bestanden';result['animation_skeletons'] = skeleton_packages
        result['asset_package_files'] += list(PROP_PACKAGES.values())
        result['delivery_props'] = 'Bestanden'
    startup = configure_startup(root,village,results)
    # Leave the editor in the verified player start world.
    require_world(unreal,root,MAP_PATH)
    return {'version':'V80','status':'Bestanden','map_package':MAP_PATH,'maps':results,
            'reciprocal_routes':{'status':'Bestanden','portal_ids':[entry['portal']['id'] for entry in data['maps']]},
            'delivery_prop_validation':prop_validation,
            'startup_map_config':startup,'gameplay_status':'Nicht ausgeführt',
            'coverage':'two real saved/reloaded editor blockouts, preserved well mission/economy and delivery route; runtime QA unexecuted'}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__);parser.add_argument('--preflight',action='store_true');parser.add_argument('--report',type=Path)
    parser.add_argument('--use-existing-map',action='store_true',help='Validate both saved V80 worlds without overwriting or reimporting')
    args = parser.parse_args(argv);root = Path(os.environ.get('ASKIR_V80_PROJECT',str(ROOT)))
    report = args.report or Path(os.environ.get('ASKIR_V80_MAP_REPORT',str(root/'Reports/V80/world_editor.json')))
    report.parent.mkdir(parents=True,exist_ok=True)
    if args.preflight:
        try: data = source_preflight(root)
        except Exception as exc:
            report.write_text(json.dumps({'status':'Fehlgeschlagen','maps':list(MAP_PATHS),'error':str(exc),'editor_status':'Nicht ausgeführt','gameplay_status':'Nicht ausgeführt'},indent=2),encoding='utf-8');raise
        result = {'status':'Bestanden','maps':list(MAP_PATHS),'quest_id':data['quest_id'],'bandit_count':len(data['bandits']),
                  'original_prop_count':len(PROP_PACKAGES),
                  'editor_status':'Nicht ausgeführt','gameplay_status':'Nicht ausgeführt','coverage':'source schema, reciprocal route geometry, canonical reward and inherited input contracts only'}
    else:
        try: import unreal
        except ModuleNotFoundError:
            report.write_text(json.dumps({'status':'Blockiert','maps':list(MAP_PATHS),'reason':'Unreal Python unavailable; no .umap/.uasset created','gameplay_status':'Nicht ausgeführt'},indent=2),encoding='utf-8');return 2
        try: result = generate(unreal,root,args.use_existing_map or os.environ.get('ASKIR_V80_VALIDATE_ONLY') == '1')
        except Exception as exc:
            report.write_text(json.dumps({'status':'Fehlgeschlagen','error':str(exc),'traceback':traceback.format_exc(),'gameplay_status':'Nicht ausgeführt'},indent=2),encoding='utf-8');raise
        unreal.log('ASKIR_V80_WORLD_VALIDATED '+','.join(MAP_PATHS))
    report.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8');return 0


if __name__ == '__main__':
    try: import unreal
    except ImportError: raise SystemExit(main())
    else: main([])
