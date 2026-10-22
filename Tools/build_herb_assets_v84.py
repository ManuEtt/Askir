"""Build original healing herb geometry with Blender; no downloaded artwork.

Run: blender -b --python Tools/build_herb_assets_v84.py
Source geometry uses metres, +X forward and +Z up. The established OBJ import
path applies exactly 100x to Unreal centimetres; FBX and GLB are retained sources.
"""
from pathlib import Path
import hashlib
import importlib.util
import json
import math

import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "ArtSource/Environment_V84"
NAME = "Askir_HealingHerbs"
MESH_NAME = "SM_AskirHealingHerbs"


def helpers():
    spec = importlib.util.spec_from_file_location("_v84_original_prop_authoring", ROOT / "Tools/build_delivery_props_v80.py")
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def leaf(api, base, angle, length, width, material):
    # A closed faceted leaf: central upper/lower ridges give actual volume.
    vertices = [(0, 0, 0), (.4*length, -width, .018), (length, 0, .045),
                (.4*length, width, .018), (.42*length, 0, .038), (.42*length, 0, .009)]
    faces = [(4, index, (index+1) % 4) for index in range(4)]
    faces += [(5, (index+1) % 4, index) for index in range(4)]
    data = bpy.data.meshes.new("Original healing leaf")
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new("Veined herb leaf", data)
    bpy.context.collection.objects.link(obj)
    obj.location = base
    obj.rotation_euler.z = angle
    api.finish_piece(obj, "Veined herb leaf", material)


def build(api):
    api.reset()
    soil = api.material("Askir_HealingHerbs_Soil", (.11, .075, .035), 0, .95)
    stem = api.material("Askir_HealingHerbs_Stem", (.085, .19, .065), 0, .86)
    dark = api.material("Askir_HealingHerbs_Leaf", (.09, .27, .105), 0, .79)
    light = api.material("Askir_HealingHerbs_YoungLeaf", (.23, .39, .13), 0, .76)
    petals = api.material("Askir_HealingHerbs_Petal", (.33, .29, .58), 0, .72)
    heart = api.material("Askir_HealingHerbs_Heart", (.68, .49, .16), 0, .82)
    # A shallow oval soil clod puts every vertex at or above the local floor.
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=1, location=(0, 0, .012))
    clod = bpy.context.object
    clod.scale = (.245, .205, .012)
    api.finish_piece(clod, "Rooted soil clod", soil)
    plants = [(0., 0., .555)] + [(math.cos(i*math.tau/6)*.13, math.sin(i*math.tau/6)*.13, .39+.027*(i % 3)) for i in range(6)]
    for index, (x, y, height) in enumerate(plants):
        lean = Vector((x*.2, y*.2, 0))
        base = Vector((x, y, .012))
        top = base + lean + Vector((0, 0, height))
        # The larger V80 props use a 4mm bevel; that collapses these 6.5mm
        # stems into coincident corners. Keep their small source cylinders clean.
        bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=.0065, depth=(top-base).length, location=(base+top)/2)
        stalk = api.finish_piece(bpy.context.object, "Healing herb stem", stem)
        stalk.rotation_euler = (top-base).to_track_quat('Z', 'Y').to_euler()
        for tier in range(4):
            anchor = base + (top-base)*(.17+tier*.175)
            direction = index*1.13+tier*1.05
            length = (.185-tier*.018) if index else (.205-tier*.02)
            for opposite in range(2):
                leaf(api, anchor, direction+opposite*math.pi, length, .038-tier*.003,
                     light if (tier+index+opposite) % 3 == 0 else dark)
        # Six substantial petals and a center on each flowering stalk.
        for petal in range(6):
            angle = petal*math.tau/6
            center = top + Vector((math.cos(angle)*.022, math.sin(angle)*.022, .003))
            bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=1, location=center)
            obj = bpy.context.object
            obj.scale = (.027, .013, .009)
            obj.rotation_euler.z = angle
            api.finish_piece(obj, "Violet healing flower petal", petals)
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=.014, location=top+Vector((0, 0, .012)))
        api.finish_piece(bpy.context.object, "Golden flower heart", heart)
    return api.combine(MESH_NAME)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    api = helpers()
    obj = build(api)
    for polygon in obj.data.polygons:
        a, b, c = (obj.data.vertices[index].co for index in polygon.vertices)
        if len(polygon.vertices) != 3 or (b-a).cross(c-a).length <= 1e-10:
            raise RuntimeError("Cannot export a degenerate original herb triangle")
    # Sources are saved before the preview adds lights, camera and backdrop.
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (NAME+".blend")))
    bpy.ops.export_scene.gltf(filepath=str(OUT / (NAME+".glb")), export_format="GLB", use_selection=True,
                              export_yup=True, export_materials="EXPORT")
    bpy.ops.export_scene.fbx(filepath=str(OUT / (NAME+".fbx")), use_selection=True, object_types={"MESH"},
                            apply_unit_scale=True, bake_anim=False, axis_forward="-Y", axis_up="Z",
                            use_mesh_modifiers=True, add_leaf_bones=False, path_mode="AUTO")
    api.write_obj(obj, OUT / (NAME+".obj"))
    for suffix in (".obj", ".mtl"):
        path = OUT/(NAME+suffix)
        path.write_text(path.read_text().replace("Original ASKIR V80", "Original ASKIR V84"))
    points = [vertex.co for vertex in obj.data.vertices]
    bounds = [[min(point[axis] for point in points) for axis in range(3)],
              [max(point[axis] for point in points) for axis in range(3)]]
    geometry = {"vertices": len(points), "triangles": len(obj.data.polygons), "materials": len(obj.data.materials),
                "bounds_metres_z_up": bounds,
                "dimensions_centimetres": [(bounds[1][axis]-bounds[0][axis])*100 for axis in range(3)]}
    # Retain an actual authoring preview; it is not a screenshot from Unreal.
    original_out = api.OUT
    api.OUT = OUT
    try:
        api.render_preview(obj, NAME)
    finally:
        api.OUT = original_out
    (OUT / "LICENSE_CC0.txt").write_text(
        "CC0 1.0 Universal\n\nOriginal ASKIR healing herb mesh, material values and authoring preview, "
        "created by Tools/build_herb_assets_v84.py. No downloaded models, textures or artwork.\n"
        "To the extent possible under law, the creators waive copyright and related rights "
        "to these original assets. https://creativecommons.org/publicdomain/zero/1.0/\n", encoding="utf-8")
    materials = {}
    for material in obj.data.materials:
        shader = material.node_tree.nodes.get("Principled BSDF")
        materials[material.name] = {"base_color": list(shader.inputs["Base Color"].default_value),
                                    "roughness": shader.inputs["Roughness"].default_value,
                                    "metallic": shader.inputs["Metallic"].default_value}
    files = {path.name: {"bytes": path.stat().st_size, "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
             for path in sorted(OUT.iterdir()) if path.is_file() and path.name in
             {NAME+suffix for suffix in (".blend", ".fbx", ".glb", ".obj", ".mtl", "_Preview.png")} | {"LICENSE_CC0.txt"}}
    metadata = {"version": "V84", "license": "CC0-1.0", "origin": "Original procedural Blender geometry and materials; no external artwork",
                "authoring_tool": "Blender "+bpy.app.version_string, "source_folder": "ArtSource/Environment_V84",
                "source_name": NAME, "mesh_name": MESH_NAME, "asset": "/Game/Askir/V84/Props/"+MESH_NAME,
                "source_units": "metres, +X forward, +Z up; GLB glTF +Y up", "unreal_obj_uniform_scale": 100,
                "geometry": geometry, "materials": materials, "files": files,
                "unreal_import": "Nicht ausgeführt", "runtime_collision": "Nicht ausgeführt"}
    destination = ROOT / "Production/Props/herb_assets_v84.json"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(metadata, indent=2)+"\n", encoding="utf-8")
    print("ASKIR V84 original healing herbs:", json.dumps(geometry))


if __name__ == "__main__":
    main()
