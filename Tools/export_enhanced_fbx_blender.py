"""Blender 4.x: blender -b --python Tools/export_enhanced_fbx_blender.py"""
from pathlib import Path
import bpy
root=Path(__file__).resolve().parents[1]
src=root/'ArtSource/Enhanced3D_V19'
dst=root/'ArtSource/EnhancedFBX_V19';dst.mkdir(exist_ok=True)
for glb in sorted(src.glob('SM_*.glb')):
 bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
 bpy.ops.import_scene.gltf(filepath=str(glb))
 meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
 if not meshes:raise RuntimeError('No meshes in '+str(glb))
 for o in meshes:
  o.scale=(100,100,100)
  bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
  bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 bpy.ops.object.select_all(action='DESELECT')
 for o in meshes:o.select_set(True)
 bpy.context.view_layer.objects.active=meshes[0]
 bpy.ops.export_scene.fbx(filepath=str(dst/(glb.stem+'.fbx')),use_selection=True,
  object_types={'MESH'},apply_unit_scale=True,bake_anim=False,axis_forward='-Y',axis_up='Z',path_mode='COPY')
 print('Exported',glb.name)
