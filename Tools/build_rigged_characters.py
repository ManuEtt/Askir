"""
Askir V4: Blender 4.x background rigging pipeline.
Run: blender -b --python Tools/build_rigged_characters.py
Outputs FBX per character, skeletal meshes with procedural skin weights,
shared bone hierarchy and nine animation Actions exported as separate FBX clips.
"""
import bpy, math
from pathlib import Path
from mathutils import Vector

PROJECT=Path(__file__).resolve().parents[1]
SOURCE=PROJECT/"ArtSource"/"Realistic3D_V1"
OUTPUT=PROJECT/"ArtSource"/"RiggedCharacters_V4"
OUTPUT.mkdir(parents=True,exist_ok=True)

# Ten implemented pipeline steps:
# 1 load character meshes; 2 standard skeleton; 3 bind mesh; 4 weights;
# 5 idle; 6 walk; 7 run; 8 attack; 9 block/hit/death; 10 FBX exports.
BONES=[
 ("root",None,(0,0,0),(0,0,.12)),
 ("pelvis","root",(0,0,.92),(0,0,1.04)),
 ("spine","pelvis",(0,0,1.04),(0,0,1.40)),
 ("chest","spine",(0,0,1.40),(0,0,1.60)),
 ("neck","chest",(0,0,1.60),(0,0,1.73)),
 ("head","neck",(0,0,1.73),(0,0,1.96)),
 ("upperarm_l","chest",(.25,0,1.55),(.36,0,1.20)),
 ("forearm_l","upperarm_l",(.36,0,1.20),(.39,0,.87)),
 ("hand_l","forearm_l",(.39,0,.87),(.39,0,.77)),
 ("upperarm_r","chest",(-.25,0,1.55),(-.36,0,1.20)),
 ("forearm_r","upperarm_r",(-.36,0,1.20),(-.39,0,.87)),
 ("hand_r","forearm_r",(-.39,0,.87),(-.39,0,.77)),
 ("thigh_l","pelvis",(.16,0,.94),(.17,0,.49)),
 ("shin_l","thigh_l",(.17,0,.49),(.17,0,.13)),
 ("foot_l","shin_l",(.17,0,.13),(.17,-.19,.09)),
 ("thigh_r","pelvis",(-.16,0,.94),(-.17,0,.49)),
 ("shin_r","thigh_r",(-.17,0,.49),(-.17,0,.13)),
 ("foot_r","shin_r",(-.17,0,.13),(-.17,-.19,.09))
]
BONE_MAP={n:(Vector(h),Vector(t)) for n,_,h,t in BONES}
def reset():
 bpy.ops.object.select_all(action='SELECT')
 bpy.ops.object.delete(use_global=False)
def build_armature():
 bpy.ops.object.armature_add(enter_editmode=False)
 rig=bpy.context.object
 rig.name="SK_AskirHumanoid"
 bpy.ops.object.mode_set(mode='EDIT')
 arm=rig.data
 for name,parent,head,tail in BONES:
  b=arm.edit_bones.new(name);b.head=head;b.tail=tail
  if parent:b.parent=arm.edit_bones[parent]
 bpy.ops.object.mode_set(mode='OBJECT')
 return rig
def closest_bone(v):
 # Bone-weight prototype: nearest segment, with strong spatial gating by body region
 z=v.z;x=v.x
 if z>=1.70:return 'head'
 if z>=1.60 and abs(x)<.22:return 'neck'
 if abs(x)>.27 and z>.74:
  side='l' if x>0 else 'r'
  return ('hand_' if z<.90 else 'forearm_' if z<1.24 else 'upperarm_')+side
 if z<.98 and abs(x)>.06:
  side='l' if x>0 else 'r'
  return ('foot_' if z<.14 else 'shin_' if z<.5 else 'thigh_')+side
 if z<1.07:return 'pelvis'
 if z<1.40:return 'spine'
 return 'chest'
def attach_mesh(mesh,rig):
 # Weight vertices in world coordinates and bind via armature modifier
 for group in mesh.vertex_groups:
  mesh.vertex_groups.remove(group)
 groups={name:mesh.vertex_groups.new(name=name) for name,*_ in BONES}
 for vert in mesh.data.vertices:
  v=mesh.matrix_world @ vert.co
  name=closest_bone(v)
  groups[name].add([vert.index],1.0,'REPLACE')
 modifier=mesh.modifiers.new("AskirSkin","ARMATURE")
 modifier.object=rig
 mesh.parent=rig
def add_clip(rig,name,frames,poses):
 bpy.context.view_layer.objects.active=rig
 rig.animation_data_create()
 action=bpy.data.actions.new(name=name)
 rig.animation_data.action=action
 for b in rig.pose.bones:
  b.rotation_mode='XYZ'
  b.rotation_euler=(0,0,0)
 for frame in frames:
  for bone_name,axis,degrees in poses.get(frame,[]):
   bone=rig.pose.bones[bone_name]
   bone.rotation_mode='XYZ'
   bone.rotation_euler[axis]=math.radians(degrees)
   bone.keyframe_insert(data_path="rotation_euler",frame=frame,group=bone_name)
  for b in rig.pose.bones:
   b.keyframe_insert(data_path="rotation_euler",frame=frame,group=b.name)
 action.use_fake_user=True
 return action
def clips(rig):
 return [
  add_clip(rig,"AN_Idle",[1,15,30],{15:[('chest',0,2)],30:[('chest',0,0)]}),
  add_clip(rig,"AN_Walk",[1,11,21],{1:[('thigh_l',0,22),('thigh_r',0,-22),('upperarm_l',0,-12),('upperarm_r',0,12)],11:[('thigh_l',0,-22),('thigh_r',0,22),('upperarm_l',0,12),('upperarm_r',0,-12)],21:[('thigh_l',0,22),('thigh_r',0,-22),('upperarm_l',0,-12),('upperarm_r',0,12)]}),
  add_clip(rig,"AN_Run",[1,7,13],{1:[('thigh_l',0,40),('thigh_r',0,-40)],7:[('thigh_l',0,-40),('thigh_r',0,40)],13:[('thigh_l',0,40),('thigh_r',0,-40)]}),
  add_clip(rig,"AN_LightAttack",[1,8,18],{1:[('upperarm_r',0,-50)],8:[('upperarm_r',0,70),('chest',2,25)],18:[('upperarm_r',0,0),('chest',2,0)]}),
  add_clip(rig,"AN_HeavyAttack",[1,12,28],{1:[('upperarm_r',0,-90)],12:[('upperarm_r',0,100),('chest',2,40)],28:[('upperarm_r',0,0),('chest',2,0)]}),
  add_clip(rig,"AN_Block",[1,8,18],{8:[('upperarm_l',0,-65),('forearm_l',0,-55)],18:[('upperarm_l',0,0),('forearm_l',0,0)]}),
  add_clip(rig,"AN_HitReact",[1,5,16],{5:[('chest',0,-22),('head',0,-15)],16:[('chest',0,0),('head',0,0)]}),
  add_clip(rig,"AN_Dodge",[1,7,18],{7:[('chest',2,35),('thigh_l',0,25)],18:[('chest',2,0),('thigh_l',0,0)]}),
  add_clip(rig,"AN_Death",[1,12,30],{12:[('root',0,35),('chest',0,25)],30:[('root',0,85),('chest',0,30)]})
 ]
def export_fbx(path,rig,meshes,action=None):
 bpy.ops.object.select_all(action='DESELECT')
 for obj in [rig]+meshes:obj.select_set(True)
 bpy.context.view_layer.objects.active=rig
 rig.animation_data.action=action if action else None
 bpy.ops.export_scene.fbx(filepath=str(path),use_selection=True,
  object_types={'ARMATURE','MESH'},add_leaf_bones=False,
  bake_anim=bool(action),bake_anim_use_all_actions=False,
  apply_unit_scale=True,axis_forward='-Y',axis_up='Z')
for role in ("Hero","Guard","Merchant","Raider"):
 reset()
 filepath=SOURCE/("SM_Character"+role+"_Preview.obj")
 if not filepath.exists():raise FileNotFoundError(filepath)
 # Blender 4.0+ OBJ importer
 bpy.ops.wm.obj_import(filepath=str(filepath))
 meshes=[o for o in bpy.context.selected_objects if o.type=='MESH']
 if not meshes:raise RuntimeError("No mesh imported: "+role)
 bpy.ops.object.select_all(action='DESELECT')
 rig=build_armature()
 for mesh in meshes:attach_mesh(mesh,rig)
 actions=clips(rig)
 export_fbx(OUTPUT/("SK_Askir"+role+".fbx"),rig,meshes,None)
 for action in actions:
  export_fbx(OUTPUT/("AN_"+role+"_"+action.name[3:]+".fbx"),rig,meshes,action)
 print("Askir rigged role",role,"actions",len(actions))
print("Askir: FBX generation complete",OUTPUT)
