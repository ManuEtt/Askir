"""UE 5.6 Editor Python: non-destructive Dornfeld integration playground.
Run in Unreal Editor Python console with:
 exec(open(r'<PROJECT>/Tools/build_dornfeld_playground_v74.py', encoding='utf-8').read())
Default PREVIEW_ONLY=True. Change to False after backup to create a NEW map.
This builds a blockout and real C++ actor instances, NOT a finished quest.
"""
import json
from pathlib import Path

PREVIEW_ONLY = True
MAP_PATH = '/Game/Askir/Prototype/V74_DornfeldPlayground'
REPORT_NAME = 'Reports/dornfeld_v74_editor_result.json'
SITES = [
    ('Mara', 'AskirNPC', (0, -500, 140)),
    ('Shard_North', 'AskirQuestTrigger', (-900, 250, 120)),
    ('Shard_East', 'AskirQuestTrigger', (850, 450, 120)),
    ('Shard_Well', 'AskirQuestTrigger', (100, 950, 120)),
    ('Guardian_1', 'AskirNPC', (-500, 1250, 140)),
    ('Guardian_2', 'AskirNPC', (500, 1250, 140)),
    ('CellarChest', 'AskirLootChest', (0, 1700, 100)),
]

def plan():
    assert len({name for name, _, _ in SITES}) == len(SITES)
    assert sum(kind == 'AskirNPC' for _, kind, _ in SITES) == 3
    return {'map': MAP_PATH, 'actors': [name for name, _, _ in SITES],
            'preview_only': PREVIEW_ONLY, 'note': 'Quest wiring and playability remain unverified'}

def build():
    import unreal
    asset_lib = unreal.EditorAssetLibrary
    if asset_lib.does_asset_exist(MAP_PATH):
        raise RuntimeError('Refusing to overwrite existing map: ' + MAP_PATH)
    actor_api = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not unreal.EditorLevelLibrary.new_level(MAP_PATH):
        raise RuntimeError('Failed to create map')
    cube = unreal.load_asset('/Engine/BasicShapes/Cube.Cube')
    if not cube:
        raise RuntimeError('Missing built-in cube mesh')
    def block(label, xyz, scale):
        actor = actor_api.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*xyz))
        actor.set_actor_label(label)
        actor.static_mesh_component.set_static_mesh(cube)
        actor.set_actor_scale3d(unreal.Vector(*scale))
        return actor
    block('WalkableGround', (0, 500, -65), (45, 45, 1))
    block('VillageSquare', (0, -500, 0), (8, 8, .25))
    block('StoneWell', (100, 950, 65), (2, 2, 1))
    for index, x in enumerate((-1600, -1150, 1150, 1600)):
        block('VillageBuilding_%02d' % index, (x, -400, 240), (4, 5, 5))
    start = actor_api.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, -1350, 160))
    start.set_actor_label('DornfeldPlayerStart')
    created = []
    for label, class_name, xyz in SITES:
        cls = getattr(unreal, class_name, None)
        if cls is None:
            raise RuntimeError('Missing compiled Unreal C++ class: ' + class_name)
        actor = actor_api.spawn_actor_from_class(cls, unreal.Vector(*xyz))
        actor.set_actor_label(label)
        if class_name == 'AskirNPC':
            actor.set_editor_property('npc_name', unreal.Text('Mara' if label == 'Mara' else 'Schattenwaechter'))
            actor.set_editor_property('b_hostile', label.startswith('Guardian'))
            if label.startswith('Guardian'):
                actor.set_editor_property('defeat_objective_id', 'DefeatShadowGuardians')
        elif class_name == 'AskirQuestTrigger':
            actor.set_editor_property('objective_id', label)
            actor.set_editor_property('amount', 1)
        elif class_name == 'AskirLootChest':
            actor.set_editor_property('persistent_loot_id', 'DornfeldCellarChest')
        created.append(label)
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError('Editor could not save the level')
    return {'map': MAP_PATH, 'created': created, 'saved': True,
            'warning': 'No validated dialogue, rewards, navigation, quest IDs or SaveGame integration'}

if __name__ == '__main__':
    result = plan() if PREVIEW_ONLY else build()
    print(json.dumps(result, indent=2, ensure_ascii=False))
