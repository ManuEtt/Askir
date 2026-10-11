"""V49: place real C++ encounter zone and spawner actors in generated Unreal maps.
Requires compiled Askir C++ module, 30 generated maps, optional enemy Blueprint classes.
DRY_RUN is intentionally True. Set False only inside Unreal Editor after backing up maps.
No enemies are spawned in editor. Without EnemyClass the spawners are inert.
"""
import json
import os
import unreal

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROFILE_FILE = os.path.join(ROOT, 'Production', 'Levels', 'encounter_profiles_v48.json')
MAP_ROOT = '/Game/Askir/WorldV42'
DRY_RUN = True
REQUIRE_ENEMY_BLUEPRINT = False


def log(message):
    unreal.log('[Askir V49] ' + str(message))


def get_class(path):
    result = unreal.load_class(None, path)
    if not result:
        raise RuntimeError('Missing compiled C++ class: ' + path)
    return result


def spawn_or_get(editor, actors_by_label, cls, label, position):
    existing = actors_by_label.get(label)
    if existing:
        if not existing.is_a(cls):
            raise RuntimeError('Label collision with wrong class: ' + label)
        return existing, False
    actor = editor.spawn_actor_from_class(cls, unreal.Vector(*position))
    if not actor:
        raise RuntimeError('Cannot spawn ' + label)
    actor.set_actor_label(label)
    actors_by_label[label] = actor
    return actor, True


def run():
    profiles = json.load(open(PROFILE_FILE, encoding='utf-8'))['profiles']
    asset_library = unreal.EditorAssetLibrary
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    zone_cls = None if DRY_RUN else get_class('/Script/Askir.AskirEncounterZone')
    spawner_cls = None if DRY_RUN else get_class('/Script/Askir.AskirEncounterSpawner')
    summary = {'maps': 0, 'missing_maps': [], 'zones_added': 0, 'spawners_added': 0,
               'unassigned_enemy_blueprints': 0, 'encounters_previewed': 0}
    for profile in profiles:
        map_name = profile['map_name']
        path = MAP_ROOT + '/' + map_name
        if not asset_library.does_asset_exist(path):
            summary['missing_maps'].append(path)
            continue
        summary['maps'] += 1
        encounters = profile['encounters']
        if DRY_RUN:
            summary['encounters_previewed'] += len(encounters)
            continue
        if not unreal.EditorLoadingAndSavingUtils.load_map(path):
            raise RuntimeError('Unable to load map: ' + path)
        by_label = {actor.get_actor_label(): actor for actor in editor.get_all_level_actors()}
        changed = False
        for item in encounters:
            encounter_id = item['encounter_id']
            x, y, z = item['position_cm']
            zone_label = 'V49_Zone_' + encounter_id
            spawner_label = 'V49_Spawner_' + encounter_id
            zone, new_zone = spawn_or_get(editor, by_label, zone_cls, zone_label, (x, y, z + 100))
            spawner, new_spawner = spawn_or_get(editor, by_label, spawner_cls, spawner_label, (x + 180, y, z + 100))
            zone.set_editor_property('encounter_id', encounter_id)
            zone.set_editor_property('b_once_per_player', True)
            spawner.set_editor_property('zone', zone)
            spawner.set_editor_property('enemies_per_wave', item['enemy_count'])
            spawner.set_editor_property('wave_count', 1)
            spawner.set_editor_property('spawn_radius', 350.0)
            class_path = item.get('blueprint_class_path')
            if class_path:
                enemy_class = unreal.load_class(None, class_path)
                if not enemy_class:
                    raise RuntimeError('Enemy Blueprint class missing: ' + class_path)
                spawner.set_editor_property('enemy_class', enemy_class)
            else:
                summary['unassigned_enemy_blueprints'] += 1
                if REQUIRE_ENEMY_BLUEPRINT:
                    raise RuntimeError('Enemy Blueprint missing for ' + encounter_id)
            summary['zones_added'] += int(new_zone)
            summary['spawners_added'] += int(new_spawner)
            changed = changed or new_zone or new_spawner
        if changed and not unreal.EditorLevelLibrary.save_current_level():
            raise RuntimeError('Unable to save map: ' + path)
    log(summary)
    if summary['missing_maps']:
        log('Generate missing maps with the V42 tool before running V49 integration.')
    return summary

if __name__ == '__main__':
    run()
