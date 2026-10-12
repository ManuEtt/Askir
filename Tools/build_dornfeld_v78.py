"""UE5.6 V78 integration: preserved V77 village logic, rigged sources and magic cues."""
from pathlib import Path
import importlib.util
import json
import os
import traceback

MAP_PATH="/Game/Askir/Maps/Dornfeld_V78"
ROOT=Path(__file__).resolve().parents[1]

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result

def import_magic_audio(unreal,root):
    tasks=[];paths=[];library=unreal.EditorAssetLibrary
    for cue in ('SpellCast','SpellImpact'):
        name='S_'+cue;path='/Game/Askir/V78/Audio/'+name;source=root/'ArtSource/Audio_V78'/ (name+'.wav')
        if not source.is_file():raise RuntimeError('Magic audio source missing: '+str(source))
        if not library.does_asset_exist(path):
            task=unreal.AssetImportTask();task.set_editor_property('filename',str(source));task.set_editor_property('destination_path','/Game/Askir/V78/Audio');task.set_editor_property('destination_name',name);task.set_editor_property('automated',True);task.set_editor_property('replace_existing',False);task.set_editor_property('save',True);tasks.append(task)
        paths.append(path)
    if tasks:unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    for path in paths:
        asset=unreal.load_asset(path)
        if not isinstance(asset,unreal.SoundWave) or float(asset.get_editor_property('duration'))<=0:raise RuntimeError('Magic sound cannot load: '+path)
        physical=root/('Content/'+path[len('/Game/'):]+'.uasset')
        if not physical.is_file() or physical.stat().st_size<128:raise RuntimeError('Sound package missing: '+str(physical))
    return paths

def validate_paths(unreal,actor_api):
    labels={actor.get_actor_label():actor for actor in actor_api.get_all_level_actors()}
    world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    start=labels['DornfeldPlayerStart'].get_actor_location()
    targets=['Mara','Dornfeld_Shard_North','Dornfeld_Shard_East','Dornfeld_Shard_Well','Dornfeld_CellarChest','Dornfeld_Guardian_1','Dornfeld_Guardian_2']
    checked=[]
    for name in targets:
        # Test the closest approach, rather than the interior of an interactive collision volume.
        location=labels[name].get_actor_location();direction=(start-location).get_safe_normal()
        endpoint=location+direction*140
        path=unreal.NavigationSystemV1.find_path_to_location_synchronously(world,start,endpoint)
        if path is None or not path.is_valid() or path.is_partial():raise RuntimeError('No complete nav path from start to '+name)
        checked.append(name)
    return {'status':'Bestanden','coverage':'Editor navigation paths to mission sources, not movement/playthrough tests','targets':checked}

def generate(unreal,root,validate_only=False):
    village=module('_askir_v77_village_for_v78',root/'Tools/build_dornfeld_v77.py');village.MAP_PATH=MAP_PATH
    importer=module('_askir_v78_character_import',root/'Tools/import_animated_assets_v78.py')
    # All source preflight precedes editor mutations.
    importer.source_preflight(root)
    animation=importer.import_assets(unreal,root)
    sounds=import_magic_audio(unreal,root)
    actor_api=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    configure_startup=village.configure_startup_maps
    if validate_only:
        if not unreal.EditorLevelLibrary.load_level(MAP_PATH):raise RuntimeError('Existing V78 map cannot load')
        result={'status':'Bestanden','map_package':MAP_PATH,'created_assets':[]}
    else:
        village.configure_startup_maps=lambda *args:{'status':'Nicht ausgeführt','reason':'waiting for V78 final validation'}
        result=village.generate(unreal,root)
        for actor in actor_api.get_all_level_actors():
            if actor.get_actor_label().startswith('Dornfeld_Guardian_'):
                playback=actor.get_editor_property('animation_playback')
                if not playback.configure_character_visual(unreal.Name('Raider')):raise RuntimeError('Guardian visual could not be configured')
        if not unreal.EditorLevelLibrary.save_current_level() or not unreal.EditorLevelLibrary.load_level(MAP_PATH):raise RuntimeError('V78 map save/reload failed')
    validation=village.validate_saved_level(unreal,actor_api,root)
    validation['navigation_paths']=validate_paths(unreal,actor_api)
    validation['asset_package_files']=list(validation.get('asset_package_files',[]))+sounds
    result['validation']=validation
    result['startup_map_config']=configure_startup(root,validation)
    result['animation_import']=animation
    result['created_assets']=list(result.get('created_assets',[]))+sounds
    result['map_file']=str(root/'Content/Askir/Maps/Dornfeld_V78.umap')
    result['gameplay_status']='Nicht ausgeführt'
    result['coverage']='Real editor imports, map reload, references and navigation paths; gameplay remains untested'
    return result

def main():
    root=Path(os.environ.get('ASKIR_V78_PROJECT',str(ROOT)));report=Path(os.environ.get('ASKIR_V78_MAP_REPORT',str(root/'Reports/V78/dornfeld_editor.json')))
    report.parent.mkdir(parents=True,exist_ok=True)
    try:
        import unreal
        if not unreal.SystemLibrary.get_engine_version().startswith('5.6.'):raise RuntimeError('UE5.6 required')
        result=generate(unreal,root,os.environ.get('ASKIR_V78_VALIDATE_ONLY')=='1')
    except Exception as exc:
        report.write_text(json.dumps({'status':'Fehlgeschlagen','error':str(exc),'traceback':traceback.format_exc(),'gameplay_status':'Nicht ausgeführt'},indent=2));raise
    report.write_text(json.dumps(result,indent=2,ensure_ascii=False),encoding='utf-8');unreal.log('ASKIR_V78_MAP_VALIDATED '+MAP_PATH)
if __name__=='__main__':main()
