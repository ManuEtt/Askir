"""V44 Unreal Editor 5.6 integration. Preview by default; requires V42 maps and compiled Askir C++.
Creates quest location triggers and encounter zones; spawners require a configured enemy Blueprint.
Never generates final art, navmesh, quest assets or enemy Blueprints.
"""
import json, os
import unreal
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA=os.path.join(ROOT,'Production','Levels','level_designs_v42.json')
DRY_RUN=True
MAP_ROOT='/Game/Askir/WorldV42'
# Configure these IDs only after the quest database has matching objectives.
ENABLE_QUEST_TRIGGERS=False
ENABLE_ENCOUNTER_SPAWNERS=True
ENEMY_BLUEPRINT_PATH=''  # e.g. /Game/Askir/Enemies/BP_ShadowGuardian.BP_ShadowGuardian_C
with open(DATA,encoding='utf-8') as stream:
    LEVELS=json.load(stream)['levels']
assets=unreal.EditorAssetLibrary
actor_system=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
quest_class=unreal.load_class(None,'/Script/Askir.AskirQuestTrigger')
zone_class=unreal.load_class(None,'/Script/Askir.AskirEncounterZone')
spawner_class=unreal.load_class(None,'/Script/Askir.AskirEncounterSpawner')
if not DRY_RUN and (not zone_class or (ENABLE_QUEST_TRIGGERS and not quest_class) or (ENABLE_ENCOUNTER_SPAWNERS and not spawner_class)):
    raise RuntimeError('Askir C++ actor classes missing. Compile in UE5.6 first.')
enemy_class=unreal.load_class(None,ENEMY_BLUEPRINT_PATH) if ENEMY_BLUEPRINT_PATH else None
if ENABLE_ENCOUNTER_SPAWNERS and not enemy_class:
    unreal.log_warning('No enemy Blueprint configured: spawners will be disabled, zones still created.')
def existing():
    return {a.get_actor_label():a for a in actor_system.get_all_level_actors()}
def spawn_once(label,cls,position,known):
    if label in known:return known[label],False
    if DRY_RUN:
        unreal.log('PREVIEW '+label);return None,False
    a=actor_system.spawn_actor_from_class(cls,unreal.Vector(*position))
    if not a:raise RuntimeError('Spawn failed: '+label)
    a.set_actor_label(label);known[label]=a
    return a,True
def process(spec):
    path=MAP_ROOT+'/'+spec['map_name']
    if not assets.does_asset_exist(path):
        unreal.log_warning('MISSING MAP '+path);return 'missing_map'
    if not DRY_RUN and not unreal.EditorLoadingAndSavingUtils.load_map(path):
        raise RuntimeError('Unable to load '+path)
    known=existing() if not DRY_RUN else {}
    count=0
    for encounter in spec['encounters']:
        ident=encounter['id']; x,y,z=encounter['position']
        zone,created=spawn_once('V44_Encounter_'+ident,zone_class,(x,y,z+100),known)
        count+=int(created)
        if zone:
            zone.set_editor_property('encounter_id',unreal.Name(ident))
            # Do not assign invented quest objective IDs.
        if ENABLE_ENCOUNTER_SPAWNERS and enemy_class:
            spawner,created=spawn_once('V44_Spawner_'+ident,spawner_class,(x+250,y,z+100),known)
            count+=int(created)
            if spawner:
                spawner.set_editor_property('zone',zone)
                spawner.set_editor_property('enemy_class',enemy_class)
                spawner.set_editor_property('enemies_per_wave',3 if encounter['kind']=='standard' else 1)
    if ENABLE_QUEST_TRIGGERS:
        for site in spec['sites']:
            if not site['mission']:continue
            x,y,z=site['position']
            trigger,created=spawn_once('V44_Quest_'+site['id'],quest_class,(x,y,z+110),known)
            count+=int(created)
            if trigger:
                trigger.set_editor_property('quest_id',unreal.Name(site['mission']))
                # ObjectiveID MUST be set by quest author; no fictitious ID assigned.
    if not DRY_RUN and count and not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError('Unable to save '+path)
    return 'preview' if DRY_RUN else 'created_'+str(count)
results={}
for spec in LEVELS:
    try:results[spec['map_name']]=process(spec)
    except Exception as exc:
        results[spec['map_name']]='ERROR '+str(exc)
        unreal.log_error(str(exc))
unreal.log('V44 integration results '+str(results))
