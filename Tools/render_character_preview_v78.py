"""Render original source previews with Blender. These are not Unreal screenshots."""
from pathlib import Path
import bpy
from mathutils import Vector

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/"ArtSource/Animated_V78/Previews"
OUT.mkdir(exist_ok=True)
for role,clip,frame in (("Hero","Idle",1),("Guard","Block",16),("Raider","LightAttack",13)):
    bpy.ops.wm.open_mainfile(filepath=str(ROOT/"ArtSource/Animated_V78"/("Askir_"+role+".blend")))
    rig=next(obj for obj in bpy.context.scene.objects if obj.type=="ARMATURE")
    rig.animation_data.action=bpy.data.actions[clip]
    scene=bpy.context.scene
    scene.frame_set(frame)
    bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,-.01))
    floor=bpy.context.object
    mat=bpy.data.materials.new("PreviewFloor")
    mat.diffuse_color=(.035,.045,.05,1)
    floor.data.materials.append(mat)
    bpy.ops.object.camera_add(location=(3.7,-5,2.5))
    camera=bpy.context.object
    camera.rotation_euler=(Vector((0,0,.95))-camera.location).to_track_quat("-Z","Y").to_euler()
    camera.data.type="ORTHO"
    camera.data.ortho_scale=2.25
    scene.camera=camera
    for location,energy,size in (((3,-4,5),750,4),((-2,3,3),500,3)):
        bpy.ops.object.light_add(type="AREA",location=location)
        lamp=bpy.context.object
        lamp.data.energy=energy
        lamp.data.shape="DISK"
        lamp.data.size=size
        lamp.rotation_euler=(Vector((0,0,1))-lamp.location).to_track_quat("-Z","Y").to_euler()
    scene.world.color=(.1,.1,.1)
    scene.render.engine="CYCLES"
    scene.cycles.device="CPU"
    scene.cycles.samples=16
    scene.cycles.use_denoising=False
    scene.render.resolution_x=512
    scene.render.resolution_y=640
    scene.render.resolution_percentage=100
    scene.render.image_settings.file_format="PNG"
    scene.render.filepath=str(OUT/(role+"_"+clip+"_Blender.png"))
    bpy.ops.render.render(write_still=True)
