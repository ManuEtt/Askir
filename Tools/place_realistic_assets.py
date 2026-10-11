# Run after import, in a level that you have saved separately.
# These props are in meters; Unreal uses centimeters.
import unreal
placements = [
    ("SM_TimberHut",(600,350,0)),
    ("SM_StoneWell",(50,450,0)),
    ("SM_StoneWall",(200,-500,0)),
    ("SM_WoodenCrate",(-200,80,0)),
    ("SM_Barrel",(-260,130,0)),
    ("SM_Campfire",(0,0,0)),
    ("SM_PineTree",(-750,-450,0)),
    ("SM_PineTree",(850,-450,0)),
    ("SM_PineTree",(-800,550,0)),
]
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for name, pos in placements:
    mesh = unreal.load_asset("/Game/Askir/Realistic3D_V1/"+name)
    if not mesh:
        unreal.log_warning("Missing imported mesh: "+name)
        continue
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*pos))
    actor.set_actor_label("AskirArt_"+name)
    actor.static_mesh_component.set_static_mesh(mesh)
unreal.log("Askir scene props placed; save your level manually.")
