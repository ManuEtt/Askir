"""UE 5.6 Editor Python: apply validated pawn combat settings, preview by default.
Run after C++ compilation and V51/V53 Blueprint creation. Back up project first.
No montage, sound or VFX assets are fabricated or assigned.
"""
import json
from pathlib import Path
import unreal
DRY_RUN=True
STRICT=True
ROOT=Path(__file__).resolve().parent.parent
MANIFEST=ROOT/'Production/Blueprints/combat_animation_profiles_v55.json'

def run():
    rows=json.loads(MANIFEST.read_text(encoding='utf-8'))['profiles']
    report={'preview':[],'configured':[],'missing':[],'errors':[]}
    lib=unreal.EditorAssetLibrary
    for row in rows:
        path=row['pawn_asset']
        if not lib.does_asset_exist(path):
            report['missing'].append(path);continue
        bp=lib.load_asset(path)
        if bp is None:
            report['missing'].append(path);continue
        cls=unreal.load_class(None,row['pawn_generated_class'])
        if cls is None:
            report['missing'].append(row['pawn_generated_class']);continue
        props={'b_hostile':row['hostile'],'detection_radius':row['detection_radius_cm'],
               'max_chase_distance':row['max_chase_distance_cm'],
               'attack_distance':row['attack_distance_cm'],
               'attack_interval':row['attack_interval_seconds'],
               'decision_interval':row['decision_interval_seconds']}
        if DRY_RUN:
            report['preview'].append({'asset':path,'properties':props});continue
        try:
            cdo=unreal.get_default_object(cls)
            # Fail visibly if a C++ property cannot be edited; never silently claim success.
            for key,val in props.items(): cdo.set_editor_property(key,val)
            # CDO edits may not persist across recompiles; verify in Editor after saving.
            if not lib.save_loaded_asset(bp): raise RuntimeError('save failed')
            report['configured'].append(path)
        except Exception as e: report['errors'].append(path+': '+str(e))
    unreal.log('[Askir V55] '+str(report))
    if STRICT and (report['missing'] or report['errors']): raise RuntimeError('V55 incomplete; see Output Log')
    return report
if __name__=='__main__': run()
