"""UE5.6 village economy integration. Produces packages only inside the real editor.

Preserves the V78 mission, visuals and animation import pipeline. The default map
changes only after V79 economy assets, actor IDs, collision and navigation survive
saving/reloading. Offline --preflight validates inputs and produces no Unreal assets.
"""
from __future__ import annotations
import argparse
import importlib.util
import json
import math
import os
from pathlib import Path
import traceback
import sys

ROOT = Path(__file__).resolve().parents[1]
MAP_PATH = '/Game/Askir/Maps/Dornfeld_V79'
MERCHANT_LABEL = 'Dornfeld_Merchant_Edda'
STATION_LABEL = 'Dornfeld_CraftingStation'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def source_preflight(root):
    catalog = module('_askir_v79_catalog', root / 'Tools/generate_village_catalog_v79.py')
    data = catalog.load_contract(root / 'Production/Economy/dornfeld_v79.json')
    generated = root / 'Source/Askir/Economy/AskirVillageEconomyCatalog.generated.inl'
    if not generated.is_file() or generated.read_text(encoding='utf-8') != catalog.generated_text(data):
        raise RuntimeError('Native village catalog is stale; run generate_village_catalog_v79.py')
    return data


def economy_assets(unreal, root, data, village):
    assets = unreal.EditorAssetLibrary
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    result = {}
    for spec in data['items']:
        path = data['asset_root'] + '/' + spec['id']
        item = unreal.load_asset(path) if assets.does_asset_exist(path) else None
        if item is None:
            factory = unreal.DataAssetFactory()
            factory.set_editor_property('data_asset_class', unreal.AskirItemData)
            item = tools.create_asset(spec['id'], data['asset_root'], unreal.AskirItemData, factory)
            if item is None: raise RuntimeError('Village item creation failed: ' + path)
            for field, value in item_properties(unreal, spec).items(): item.set_editor_property(field, value)
            if not assets.save_loaded_asset(item, only_if_is_dirty=False): raise RuntimeError('Village item cannot save: ' + path)
        validate_item(unreal, item, spec)
        village.assert_saved_package(root, path)
        result[spec['id']] = item
    return result


def item_properties(unreal, spec):
    return {'item_id': unreal.Name(spec['id']), 'display_name': unreal.Text(spec['name']),
            'description': unreal.Text(spec['description']), 'item_type': getattr(unreal.AskirItemType, spec['type'].upper()),
            'max_stack_size': spec['stack'], 'weight': spec['weight'], 'value': spec['value'],
            'damage': spec['damage'], 'heal_amount': spec['heal'], 'attack_range': spec['attack_range'],
            'attack_cooldown': spec['attack_cooldown']}


def validate_item(unreal, item, spec):
    if not isinstance(item, unreal.AskirItemData): raise RuntimeError('Village item package has wrong asset type')
    for field, expected in item_properties(unreal, spec).items():
        actual = item.get_editor_property(field)
        if isinstance(expected, (int, float)):
            good = abs(float(actual) - float(expected)) < 0.0001
        else: good = str(actual) == str(expected)
        if not good: raise RuntimeError('Existing village item conflicts with catalog: ' + spec['id'] + '.' + field)


def place_economy(unreal, actor_api, data, items):
    created = []
    labels = {actor.get_actor_label() for actor in actor_api.get_all_level_actors()}
    def spawn(cls, label, position):
        if label in labels: raise RuntimeError('Duplicate economy actor: ' + label)
        actor = actor_api.spawn_actor_from_class(cls, unreal.Vector(*position), unreal.Rotator(0, 0, 0))
        if actor is None: raise RuntimeError('Economy actor creation failed: ' + label)
        actor.set_actor_label(label);actor.set_editor_property('tags', [unreal.Name('ASKIR_V79')])
        created.append(label);labels.add(label)
        return actor
    def visual(actor, shape, scale):
        mesh = unreal.load_asset('/Engine/BasicShapes/' + shape + '.' + shape)
        if mesh is None: raise RuntimeError('Engine primitive unavailable: ' + shape)
        component = actor.get_editor_property('mesh');component.set_static_mesh(mesh)
        component.set_relative_scale3d(unreal.Vector(*scale));component.set_collision_profile_name('BlockAll')
        return component
    merchant = spawn(unreal.AskirMerchantActor, MERCHANT_LABEL, data['merchant_position'])
    merchant.set_editor_property('merchant_name', unreal.Text(data['merchant_name']))
    merchant.set_editor_property('interaction_range', 250.0)
    visual(merchant, 'Cylinder', (.65, .65, 1.8))
    shop = merchant.get_editor_property('merchant')
    shop.set_editor_property('merchant_id', unreal.Name(data['merchant_id']))
    shop.set_editor_property('merchant_gold', data['merchant_gold'])
    shop.set_editor_property('sell_back_ratio', data['sell_back_ratio'])
    stock = []
    for spec in data['items']:
        entry = unreal.AskirShopItem()
        for field, value in (('item_id', unreal.Name(spec['id'])), ('base_price', spec['value']), ('stock', spec['stock']), ('max_stock', spec['max_stock'])):
            entry.set_editor_property(field, value)
        stock.append(entry)
    shop.set_editor_property('inventory', stock)
    station = spawn(unreal.AskirCraftingStationActor, STATION_LABEL, data['station_position'])
    station.set_editor_property('station_name', unreal.Text(data['station_name']))
    station.set_editor_property('interaction_range', 250.0)
    visual(station, 'Cube', (2, 1, .65))
    timber = unreal.load_asset('/Game/Askir/V77/Materials/M_DornfeldTimber')
    if timber is None: raise RuntimeError('Workbench material unavailable')
    station.get_editor_property('mesh').set_material(0, timber)
    for spec in data['resource_pickups']:
        pickup = spawn(unreal.AskirLootPickup, spec['id'], spec['position'])
        pickup.set_editor_property('item', items[spec['item_id']]);pickup.set_editor_property('quantity', spec['quantity'])
        pickup.set_editor_property('persistent_loot_id', unreal.Name(spec['id']))
        component = pickup.get_editor_property('visual')
        component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Sphere.Sphere'))
        component.set_relative_scale3d(unreal.Vector(.55, .55, .55));component.set_material(0, timber)
    return created



def approach_point(unreal, start, location, distance):
    # Compute in Python rather than relying on engine-version-specific Vector methods.
    delta = [start.x - location.x, start.y - location.y, start.z - location.z]
    length = math.sqrt(sum(value * value for value in delta))
    if not math.isfinite(length) or length <= 0: raise RuntimeError('Navigation approach has invalid/coincident endpoints')
    return unreal.Vector(location.x + delta[0] / length * distance,
                         location.y + delta[1] / length * distance,
                         location.z + delta[2] / length * distance)


def validate_mission_paths(unreal, actor_api):
    labels = {actor.get_actor_label(): actor for actor in actor_api.get_all_level_actors()}
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    start = labels['DornfeldPlayerStart'].get_actor_location()
    targets = ['Mara', 'Dornfeld_Shard_North', 'Dornfeld_Shard_East', 'Dornfeld_Shard_Well', 'Dornfeld_CellarChest', 'Dornfeld_Guardian_1', 'Dornfeld_Guardian_2']
    for label in targets:
        endpoint = approach_point(unreal, start, labels[label].get_actor_location(), 140)
        path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, endpoint)
        if path is None or not path.is_valid() or path.is_partial(): raise RuntimeError('No complete mission navigation path to ' + label)
    return {'status': 'Bestanden', 'targets': targets, 'coverage': 'editor navigation path queries; gameplay not executed'}


def validate_economy(unreal, actor_api, root, data, village):
    actors = actor_api.get_all_level_actors()
    labels = {actor.get_actor_label(): actor for actor in actors}
    if len(labels) != len(actors): raise RuntimeError('Saved map contains duplicate actor labels')
    for label in [MERCHANT_LABEL, STATION_LABEL] + [entry['id'] for entry in data['resource_pickups']]:
        if label not in labels: raise RuntimeError('Economy actor missing after reload: ' + label)
        if not unreal.AskirEditorLevelTools.has_walkable_ground_below(labels[label]): raise RuntimeError('Economy actor is not above walkable collision: ' + label)
    merchant = labels[MERCHANT_LABEL]
    if not isinstance(merchant, unreal.AskirMerchantActor): raise RuntimeError('Merchant native class lost')
    shop = merchant.get_editor_property('merchant')
    if str(shop.get_editor_property('merchant_id')) != data['merchant_id']: raise RuntimeError('Persistent merchant ID lost')
    if shop.get_editor_property('merchant_gold') != data['merchant_gold']: raise RuntimeError('Initial merchant gold differs')
    if abs(float(shop.get_editor_property('sell_back_ratio')) - data['sell_back_ratio']) > 0.0001: raise RuntimeError('Merchant price ratio changed')
    actual_stock = shop.get_editor_property('inventory')
    if len(actual_stock) != len(data['items']): raise RuntimeError('Merchant stock catalog incomplete')
    for entry, spec in zip(actual_stock, data['items']):
        for field, expected in (('item_id', spec['id']), ('base_price', spec['value']), ('stock', spec['stock']), ('max_stock', spec['max_stock'])):
            if str(entry.get_editor_property(field)) != str(expected): raise RuntimeError('Merchant stock differs: ' + spec['id'] + '.' + field)
    for label in (MERCHANT_LABEL, STATION_LABEL):
        component = labels[label].get_editor_property('mesh')
        if component.get_editor_property('static_mesh') is None or str(component.get_collision_profile_name()) != 'BlockAll': raise RuntimeError('Economy visual/collision lost: ' + label)
        if float(labels[label].get_editor_property('interaction_range')) != 250: raise RuntimeError('Economy interaction range changed')
    if not isinstance(labels[STATION_LABEL], unreal.AskirCraftingStationActor): raise RuntimeError('Workbench native class lost')
    for spec in data['resource_pickups']:
        actor = labels[spec['id']]
        if str(actor.get_editor_property('persistent_loot_id')) != spec['id'] or int(actor.get_editor_property('quantity')) != spec['quantity']: raise RuntimeError('Resource pickup receipt/quantity lost')
        item = actor.get_editor_property('item')
        if item is None or str(item.get_editor_property('item_id')) != spec['item_id']: raise RuntimeError('Resource item reference lost')
        if not str(actor.get_editor_property('dornfeld_shard_id')) in ('None', ''): raise RuntimeError('Resource pickup incorrectly advances well-shard mission')
    packages = []
    for spec in data['items']:
        path = data['asset_root'] + '/' + spec['id'];validate_item(unreal, unreal.load_asset(path), spec)
        village.assert_saved_package(root, path);packages.append(path)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    start = labels['DornfeldPlayerStart'].get_actor_location()
    checked = []
    for label in [MERCHANT_LABEL, STATION_LABEL] + [entry['id'] for entry in data['resource_pickups']]:
        endpoint = approach_point(unreal, start, labels[label].get_actor_location(), 190)
        path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, endpoint)
        if path is None or not path.is_valid() or path.is_partial(): raise RuntimeError('No complete navigation path to economy actor: ' + label)
        checked.append(label)
    return {'status': 'Bestanden', 'asset_package_files': packages, 'merchant_id': data['merchant_id'],
            'navigation_targets': checked, 'recipe_count': len(data['recipes']),
            'coverage': 'saved editor references, stock, item effect parameters, collision and navigation; runtime transactions not executed'}


def configure_startup(root, village, validation):
    if validation.get('economy', {}).get('status') != 'Bestanden' or validation.get('navigation_paths', {}).get('status') != 'Bestanden':
        raise RuntimeError('Startup map requires all saved V79 economy and mission navigation validations')
    return village.configure_startup_maps(root, validation)


def generate(unreal, root, validate_only=False):
    data = source_preflight(root)
    if not unreal.SystemLibrary.get_engine_version().startswith('5.6.'): raise RuntimeError('UE5.6 required')
    for name in ('AskirMerchantActor', 'AskirCraftingStationActor', 'AskirShopItem'):
        if getattr(unreal, name, None) is None: raise RuntimeError('Native economy class unavailable; compile AskirEditor: ' + name)
    village = module('_askir_v77_village_for_v79', root / 'Tools/build_dornfeld_v77.py');village.MAP_PATH = MAP_PATH
    base = module('_askir_v78_village_for_v79', root / 'Tools/build_dornfeld_v78.py');base.MAP_PATH = MAP_PATH
    base.validate_paths = validate_mission_paths
    actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if validate_only:
        if not unreal.EditorLevelLibrary.load_level(MAP_PATH): raise RuntimeError('Existing V79 map cannot load')
        result = {'status': 'Bestanden', 'map_package': MAP_PATH, 'created_actors': [], 'created_assets': []}
    else:
        # A V78 wrapper must never configure startup before the later V79 economy validation.
        original_module = base.module
        def wrapped_module(name, path):
            result_module = original_module(name, path)
            if Path(path).name == 'build_dornfeld_v77.py':
                result_module.configure_startup_maps = lambda *args: {'status': 'Nicht ausgeführt', 'reason': 'waiting for V79 final validation'}
            return result_module
        base.module = wrapped_module
        result = base.generate(unreal, root)
        items = economy_assets(unreal, root, data, village)
        result['created_actors'] += place_economy(unreal, actor_api, data, items)
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        if not unreal.AskirEditorLevelTools.build_editor_navigation(world): raise RuntimeError('Economy collision navigation rebuild failed')
        if not unreal.EditorLevelLibrary.save_current_level() or not unreal.EditorLevelLibrary.load_level(MAP_PATH): raise RuntimeError('V79 map save/reload failed')
    validation = village.validate_saved_level(unreal, actor_api, root)
    validation['navigation_paths'] = base.validate_paths(unreal, actor_api)
    validation['economy'] = validate_economy(unreal, actor_api, root, data, village)
    validation['asset_package_files'] += validation['economy']['asset_package_files']
    # Revalidate inherited magic cues on validate-only runs, too.
    for cue in ('SpellCast', 'SpellImpact'):
        package = '/Game/Askir/V78/Audio/S_' + cue
        if not isinstance(unreal.load_asset(package), unreal.SoundWave): raise RuntimeError('Required V78 spell cue lost')
        village.assert_saved_package(root, package);validation['asset_package_files'].append(package)
    result['validation'] = validation
    result['startup_map_config'] = configure_startup(root, village, validation)
    result['map_file'] = str(root / 'Content/Askir/Maps/Dornfeld_V79.umap')
    result['created_assets'] = list(dict.fromkeys(result.get('created_assets', []) + validation['asset_package_files']))
    result['gameplay_status'] = 'Nicht ausgeführt'
    result['coverage'] = 'real V79 editor map save/reload with mission and starter economy; gameplay/playthrough remain untested'
    result['remaining'] = ['UE runtime trade/crafting/save-load QA and full campaign/content work']
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__);parser.add_argument('--preflight', action='store_true');parser.add_argument('--report', type=Path)
    args = parser.parse_args(argv)
    root = Path(os.environ.get('ASKIR_V79_PROJECT', str(ROOT)))
    report = args.report or Path(os.environ.get('ASKIR_V79_MAP_REPORT', str(root / 'Reports/V79/dornfeld_editor.json')))
    report.parent.mkdir(parents=True, exist_ok=True)
    if args.preflight:
        data = source_preflight(root)
        result = {'status': 'Bestanden', 'map_package': MAP_PATH, 'item_count': len(data['items']), 'recipe_count': len(data['recipes']), 'coverage': 'input schema/catalog synchronization only', 'editor_status': 'Nicht ausgeführt', 'gameplay_status': 'Nicht ausgeführt'}
        report.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8');return 0
    try:
        import unreal
    except ModuleNotFoundError:
        report.write_text(json.dumps({'status': 'Blockiert', 'map_package': MAP_PATH, 'reason': 'Unreal Python runtime unavailable; no .umap or .uasset created', 'gameplay_status': 'Nicht ausgeführt'}, indent=2), encoding='utf-8');return 2
    try:
        result = generate(unreal, root, os.environ.get('ASKIR_V79_VALIDATE_ONLY') == '1')
    except Exception as exc:
        report.write_text(json.dumps({'status': 'Fehlgeschlagen', 'map_package': MAP_PATH, 'error': str(exc), 'traceback': traceback.format_exc(), 'gameplay_status': 'Nicht ausgeführt'}, indent=2), encoding='utf-8');raise
    report.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8');unreal.log('ASKIR_V79_MAP_VALIDATED ' + MAP_PATH)
    return 0

if __name__ == '__main__':
    # UE commandlets own sys.argv; use ASKIR_V79_* environment values inside the editor.
    try:
        import unreal
    except ImportError:
        raise SystemExit(main())
    else:
        main([])
