"""Run inside Unreal Editor after importing meshes, with a test level open."""
import unreal
items=[('SM_TimberHut',(600,350,0)),('SM_StoneWell',(50,450,0)),('SM_StoneWall',(200,-500,0)),
 ('SM_WoodenCrate',(-200,80,0)),('SM_Barrel',(-260,130,0)),('SM_Campfire',(0,0,0)),
 ('SM_PineTree',(-750,-450,0)),('SM_PineTree',(850,-450,0)),('SM_PineTree',(-800,550,0)),
 ('SM_CharacterGuard_Preview',(300,120,0)),('SM_CharacterMerchant_Preview',(70,410,0)),
 ('SM_CharacterRaider_Preview',(-430,-250,0))]
subsystem=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for name,xyz in items:
 mesh=unreal.load_asset('/Game/Askir/Enhanced3D_V19/'+name)
 if not mesh:unreal.log_warning('Missing mesh: '+name);continue
 actor=subsystem.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*xyz))
 actor.set_actor_label('AskirV19_'+name);actor.static_mesh_component.set_static_mesh(mesh)
unreal.log('Askir V19: placed props; save level manually')
