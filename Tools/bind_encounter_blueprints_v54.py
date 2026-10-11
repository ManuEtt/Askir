"""V54: Apply validated encounter enemy classes to V49 spawners in 30 maps.
Requires UE5.6 editor, compiled Askir module, generated maps, V51/V53 blueprints,
and placed V49 encounter actors. Safe preview by default.
"""
import json
from pathlib import Path
import unreal

DRY_RUN = True
STRICT = True
ROOT = Path(__file__).resolve().parent.parent
MAP_ROOT = '/Game/Askir/WorldV42'
BINDINGS = ROOT / 'Production/Levels/encounter_blueprint_bindings_v54.json'


def run():
    rows = json.loads(BINDINGS.read_text(encoding='utf-8'))['encounters']
    by_map = {}
    for row in rows:
        by_map.setdefault(row['map_name'], []).append(row)
    assets = unreal.EditorAssetLibrary
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    report = {'preview': 0, 'configured': 0, 'missing_maps': [],
              'missing_spawners': [], 'missing_classes': [], 'errors': []}
    for map_name, encounters in by_map.items():
        path = MAP_ROOT + '/' + map_name
        if not assets.does_asset_exist(path):
            report['missing_maps'].append(path)
            continue
        # In preview mode do not load maps or mutate editor state.
        if DRY_RUN:
            for row in encounters:
                if not unreal.load_class(None, row['enemy_class_path']):
                    report['missing_classes'].append(row['enemy_class_path'])
                else:
                    report['preview'] += 1
            continue
        if not unreal.EditorLoadingAndSavingUtils.load_map(path):
            report['errors'].append('Cannot load map: ' + path)
            continue
        actors = {a.get_actor_label(): a for a in editor.get_all_level_actors()}
        changed = False
        for row in encounters:
            label = 'V49_Spawner_' + row['encounter_id']
            spawner = actors.get(label)
            enemy_cls = unreal.load_class(None, row['enemy_class_path'])
            if spawner is None:
                report['missing_spawners'].append(map_name + ':' + label)
                continue
            if enemy_cls is None:
                report['missing_classes'].append(row['enemy_class_path'])
                continue
            try:
                if not spawner.is_a(unreal.load_class(None, '/Script/Askir.AskirEncounterSpawner')):
                    raise TypeError('Actor is not AskirEncounterSpawner: ' + label)
                previous = spawner.get_editor_property('enemy_class')
                if previous != enemy_cls:
                    spawner.set_editor_property('enemy_class', enemy_cls)
                    changed = True
                report['configured'] += 1
            except Exception as exc:
                report['errors'].append(label + ': ' + str(exc))
        if changed and not unreal.EditorLevelLibrary.save_current_level():
            report['errors'].append('Cannot save: ' + map_name)
    unreal.log('[Askir V54] ' + str(report))
    if STRICT and any(report[k] for k in ('missing_maps','missing_spawners','missing_classes','errors')):
        raise RuntimeError('Incomplete V54 binding; see Output Log')
    return report

if __name__ == '__main__':
    run()
