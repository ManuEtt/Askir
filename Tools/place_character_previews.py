# Unreal Editor Python; run after import_realistic_assets.py
# Preview actors are static. They do not replace existing gameplay characters.
import unreal
names = ["Hero","Guard","Merchant","Raider"]
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for i,role in enumerate(names):
    mesh=unreal.load_asset("/Game/Askir/Realistic3D_V1/SM_Character"+role+"_Preview")
    if not mesh:
        unreal.log_warning("Missing "+role+" preview mesh")
        continue
    actor=subsystem.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(i*200,0,0))
    actor.set_actor_label("Askir_CharacterPreview_"+role)
    actor.static_mesh_component.set_static_mesh(mesh)
unreal.log("Placed four static character previews. Save level manually.")
