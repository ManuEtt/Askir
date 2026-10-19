"""Create original ASKIR delivery props. Run Blender -b --python this_file.py.

All dimensions are metres, +X points towards the cart handles and +Z is up.
The OBJ writer preserves this basis; the Unreal importer applies scale 100.
"""
from pathlib import Path
import hashlib
import json
import math
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'ArtSource/Delivery_V80'
NAMES = ('SM_AskirDeliveryCart', 'SM_AskirSupplyCrate')


def reset():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 1.0
    bpy.context.preferences.filepaths.save_version = 0


def material(name, color, metal=0., roughness=.8):
    value = bpy.data.materials.new(name)
    value.diffuse_color = (*color, 1)
    value.use_nodes = True
    shader = value.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value = (*color, 1)
    shader.inputs['Metallic'].default_value = metal
    shader.inputs['Roughness'].default_value = roughness
    return value


def finish_piece(obj, name, mat, bevel=0.):
    obj.name = name
    obj.data.materials.append(mat)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if bevel:
        modifier = obj.modifiers.new('Small worn edges', 'BEVEL')
        modifier.width = bevel
        modifier.segments = 1
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    return obj


def box(name, center, size, mat, bevel=.008):
    bpy.ops.mesh.primitive_cube_add(size=1, location=center)
    obj = bpy.context.object
    obj.dimensions = size
    return finish_piece(obj, name, mat, bevel)


def beam(name, start, end, width, depth, mat, bevel=.006):
    start, end = Vector(start), Vector(end)
    obj = box(name, (start+end)/2, (width, depth, (end-start).length), mat, bevel)
    obj.rotation_euler = (end-start).to_track_quat('Z', 'Y').to_euler()
    return obj


def cylinder(name, center, radius, depth, mat, axis=(0, 0, 1), sides=12):
    bpy.ops.mesh.primitive_cylinder_add(vertices=sides, radius=radius, depth=depth, location=center)
    obj = finish_piece(bpy.context.object, name, mat, .004)
    obj.rotation_euler = Vector(axis).to_track_quat('Z', 'Y').to_euler()
    return obj


def ring(name, center, radius, thickness, mat, axis=(0, 0, 1)):
    bpy.ops.mesh.primitive_torus_add(major_radius=radius, minor_radius=thickness,
                                  major_segments=20, minor_segments=6, location=center)
    obj = finish_piece(bpy.context.object, name, mat)
    obj.rotation_euler = Vector(axis).to_track_quat('Z', 'Y').to_euler()
    return obj


def combine(name):
    bpy.ops.object.select_all(action='DESELECT')
    objects = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = name
    bpy.context.scene.cursor.location = (0, 0, 0)
    bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    modifier = obj.modifiers.new('Actual source triangles', 'TRIANGULATE')
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.data.update()
    return obj


def make_cart():
    oak = material('Askir_OldOak', (.25, .125, .052))
    fresh = material('Askir_CutOak', (.39, .235, .115))
    dark = material('Askir_Heartwood', (.13, .065, .027))
    iron = material('Askir_WroughtIron', (.075, .092, .092), .72, .45)
    leather = material('Askir_LeatherGrip', (.105, .045, .023), 0, .88)
    # A handcart with individually separated planks, open bed and two spoked wheels.
    for y in (-.43, .43):
        box('Undercarriage runner', (-.22, y, .58), (1.67, .095, .13), dark)
        beam('Long handle', (.25, y, .61), (1.85, y*.91, .76), .075, .075, oak)
        beam('Leather grip', (1.57, y*.92, .735), (1.88, y*.91, .765), .088, .088, leather)
    for i in range(7):
        box('Bed plank', (-.24, -.495+i*.165, .70), (1.72, .155, .095), fresh if i%3 == 0 else oak)
    for x in (-1.04, .57):
        for y in (-.58, .58):
            box('Corner upright', (x, y, .92), (.085, .085, .67), dark)
            box('Corner iron cap', (x, y, 1.255), (.094, .094, .055), iron, .004)
    for level in range(3):
        z = .84+level*.165
        for y in (-.58, .58):
            box('Sideboard', (-.235, y, z), (1.72, .055, .135), oak if level%2 else fresh)
        box('Rear board', (-1.06, 0, z), (.055, 1.19, .135), oak)
    for y in (-.618, .618):
        for x in (-.88, .41):
            box('Side iron strap', (x, y, 1.00), (.045, .018, .56), iron, .002)
            for z in (.81, 1.16):
                cylinder('Rivet', (x, y*1.018, z), .017, .017, iron, (0, 1, 0), 8)
    cylinder('Iron axle', (-.31, 0, .46), .052, 1.53, iron, (0, 1, 0))
    for y in (-.75, .75):
        center = Vector((-.31, y, .46))
        ring('Wooden wheel rim', center, .405, .045, oak, (0, 1, 0))
        ring('Iron tyre', center, .442, .017, iron, (0, 1, 0))
        cylinder('Wooden wheel hub', center, .10, .16, dark, (0, 1, 0))
        cylinder('Iron hub pin', center+Vector((0, math.copysign(.095, y), 0)), .055, .047, iron, (0, 1, 0))
        for j in range(10):
            direction = Vector((math.cos(j*math.tau/10), 0, math.sin(j*math.tau/10)))
            beam('Wheel spoke', center+direction*.075, center+direction*.39, .045, .055, fresh, .003)
    # The ground contact matches the iron tyre, no below-floor vertices.
    return combine(NAMES[0])


def make_crate():
    oak = material('Askir_CrateOak', (.32, .18, .075))
    fresh = material('Askir_CrateLightPlank', (.46, .29, .14))
    dark = material('Askir_CrateEndgrain', (.18, .085, .033))
    rope = material('Askir_HempRope', (.47, .35, .19))
    iron = material('Askir_CrateNails', (.08, .10, .10), .7, .5)
    for i in range(6):
        y = -.3225+i*.129
        for z in (.045, .705):
            box('Lid and base plank', (0, y, z), (.95, .119, .075), fresh if i%2 else oak, .005)
    for y in (-.365, .365):
        for i in range(5):
            box('Long wall plank', (-.38+i*.19, y, .375), (.18, .06, .59), oak if i%2 else fresh, .005)
        for z in (.12, .63):
            box('Long wall batten', (0, y*1.055, z), (1.0, .035, .085), dark, .004)
        beam('Diagonal side brace', (-.405, y*1.07, .17), (.405, y*1.07, .59), .075, .04, fresh, .004)
    for x in (-.465, .465):
        for i in range(4):
            box('End wall plank', (x, -.2745+i*.183, .375), (.06, .174, .59), oak if i%2 else fresh, .005)
        for z in (.12, .63):
            box('End batten', (x*1.045, 0, z), (.035, .76, .085), dark, .004)
    # Two closed rectangular rope bands, softened at corners by small rope spheres.
    for x in (-.26, .26):
        corners = [(x, -.407, .035), (x, .407, .035), (x, .407, .755), (x, -.407, .755)]
        for i in range(4):
            start, end = Vector(corners[i]), Vector(corners[(i+1)%4])
            cylinder('Hemp binding', (start+end)/2, .013, (end-start).length, rope, end-start, 8)
            bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=4, radius=.013, location=start)
            finish_piece(bpy.context.object, 'Rope corner', rope)
    for y in (-.405, .405):
        for x in (-.41, .41):
            for z in (.12, .63):
                cylinder('Square iron nail', (x, y, z), .012, .014, iron, (0, 1, 0), 4)
    return combine(NAMES[1])


def write_obj(obj, path):
    mesh = obj.data
    mats = list(mesh.materials)
    lines = ['# Original ASKIR V80. Metres, +X forward, +Z up.', 'mtllib '+path.with_suffix('.mtl').name, 'o '+obj.name]
    for v in mesh.vertices:
        lines.append('v {:.9f} {:.9f} {:.9f}'.format(*v.co))
    for face in mesh.polygons:
        lines.append('vn {:.9f} {:.9f} {:.9f}'.format(*face.normal))
    previous = None
    for index, face in enumerate(mesh.polygons):
        if len(face.vertices) != 3:
            raise RuntimeError('Source must contain actual triangles')
        if face.material_index != previous:
            previous = face.material_index
            lines.append('usemtl '+mats[previous].name)
        lines.append('f '+' '.join(str(v+1)+'//'+str(index+1) for v in face.vertices))
    path.write_text('\n'.join(lines)+'\n', encoding='utf-8')
    output = ['# Original ASKIR V80 texture-free materials']
    for mat in mats:
        shader = mat.node_tree.nodes.get('Principled BSDF')
        color = shader.inputs['Base Color'].default_value
        output.extend(['newmtl '+mat.name, 'Kd {:.6f} {:.6f} {:.6f}'.format(*color[:3]), 'Ka 0.02 0.02 0.02',
                       'Ks 0.12 0.12 0.12', 'Ns 32', 'd 1.0', 'illum 2',
                       'Pr {:.5f}'.format(shader.inputs['Roughness'].default_value),
                       'Pm {:.5f}'.format(shader.inputs['Metallic'].default_value), ''])
    path.with_suffix('.mtl').write_text('\n'.join(output), encoding='utf-8')


def render_preview(obj, name):
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 32
    scene.cycles.use_denoising = False
    scene.render.resolution_x = 960
    scene.render.resolution_y = 720
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.render.filepath = str(OUT/(name+'_Preview.png'))
    scene.world.color = (.08, .08, .08)
    ground = material('PreviewGround', (.055, .07, .075))
    box('Preview ground', (0, 0, -.04), (200, 200, .06), ground, 0)
    focus = Vector((.25, 0, .65)) if 'Cart' in name else Vector((0, 0, .36))
    bpy.ops.object.camera_add(location=(4.8, -5.4, 3.5) if 'Cart' in name else (2.5, -3.4, 2.3))
    camera = bpy.context.object
    camera.rotation_euler = (focus-camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = 3.7 if 'Cart' in name else 1.85
    scene.camera = camera
    for location, energy, size in (((-3, -4, 7), 950, 5), ((4, 3, 4), 600, 4)):
        bpy.ops.object.light_add(type='AREA', location=location)
        light = bpy.context.object
        light.data.energy = energy
        light.data.shape = 'DISK'
        light.data.size = size
        light.rotation_euler = (focus-light.location).to_track_quat('-Z', 'Y').to_euler()
    bpy.ops.render.render(write_still=True)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    records = {}
    for name, builder in zip(NAMES, (make_cart, make_crate)):
        reset()
        obj = builder()
        # Sources contain only the delivery mesh; preview camera/ground are added later.
        bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
        bpy.ops.export_scene.gltf(filepath=str(OUT/(name+'.glb')), export_format='GLB',
                                  use_selection=True, export_yup=True, export_materials='EXPORT')
        write_obj(obj, OUT/(name+'.obj'))
        coords = [v.co for v in obj.data.vertices]
        bounds = [[min(v[i] for v in coords) for i in range(3)], [max(v[i] for v in coords) for i in range(3)]]
        records[name] = {'vertices': len(obj.data.vertices), 'triangles': len(obj.data.polygons),
                         'materials': len(obj.data.materials), 'bounds_metres_z_up': bounds,
                         'dimensions_centimetres': [(bounds[1][i]-bounds[0][i])*100 for i in range(3)]}
        render_preview(obj, name)
    license_text = ('CC0 1.0 Universal\n\nOriginal ASKIR procedural 3D props generated with the project-owned '
                    'Tools/build_delivery_props_v80.py; no downloaded models, textures or external artwork.\n'
                    'To the extent possible under law, the creators have waived all copyright and related '
                    'or neighboring rights to these original mesh, material and preview assets.\n'
                    'https://creativecommons.org/publicdomain/zero/1.0/\n')
    (OUT/'LICENSE_CC0.txt').write_text(license_text, encoding='utf-8')
    sources = {path.name: {'bytes': path.stat().st_size, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
               for path in sorted(OUT.iterdir()) if path.is_file() and path.name != 'PROVENANCE.json'}
    (OUT/'PROVENANCE.json').write_text(json.dumps({'version': 'V80', 'license': 'CC0-1.0',
        'origin': 'Original project-generated Blender geometry; no external assets', 'authoring_tool': bpy.app.version_string,
        'source_units': 'metres, +X forward, +Z up; GLB uses glTF +Y up',
        'unreal_obj_uniform_scale': 100, 'props': records, 'files': sources}, indent=2)+'\n', encoding='utf-8')
    print('ASKIR V80 original props created:', json.dumps(records))


if __name__ == '__main__':
    main()
