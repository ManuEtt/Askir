"""Unreal Editor: configure AI controller Blueprint defaults (NOT pawn defaults).
Run V51/V53 setup first. Preview-only by default. Requires UE 5.6 Editor Python.
"""
import json
from pathlib import Path
import unreal
DRY_RUN = True
STRICT = True
ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / 'Production/Blueprints/combat_animation_profiles_v55.json'

def run():
    rows = json.loads(MANIFEST.read_text(encoding='utf-8'))['profiles']
    report = {'preview': [], 'configured': [], 'missing': [], 'errors': []}
    lib = unreal.EditorAssetLibrary
    for row in rows:
        path = row['controller_asset']
        pawn_path = row['pawn_asset']
        if not lib.does_asset_exist(path) or not lib.does_asset_exist(pawn_path):
            report['missing'].append({'controller': path, 'pawn': pawn_path})
            continue
        bp = lib.load_asset(path)
        if not bp:
            report['missing'].append(path)
            continue
        controller_class = bp.generated_class()
        if not controller_class:
            report['missing'].append(path + ': generated class unavailable')
            continue
        # These properties belong to AAskirEnemyAIController, not AAskirCharacter.
        props = {
            'b_hostile_to_player': row['hostile'],
            'sight_range': row['detection_radius_cm'],
            'max_chase_from_home': row['max_chase_distance_cm'],
            'attack_distance': row['attack_distance_cm'],
            'attack_interval': row['attack_interval_seconds'],
            'decision_interval': row['decision_interval_seconds'],
        }
        if DRY_RUN:
            report['preview'].append({'controller': path, 'properties': props})
            continue
        try:
            cdo = unreal.get_default_object(controller_class)
            for key, value in props.items():
                cdo.set_editor_property(key, value)
            if not lib.save_loaded_asset(bp):
                raise RuntimeError('Could not save AI Blueprint')
            report['configured'].append(path)
        except Exception as exc:
            report['errors'].append(path + ': ' + str(exc))
    unreal.log('[Askir V60 audit] ' + json.dumps(report, ensure_ascii=False))
    if STRICT and (report['missing'] or report['errors']):
        raise RuntimeError('Combat profile setup incomplete; see Output Log')
    return report

if __name__ == '__main__':
    run()
