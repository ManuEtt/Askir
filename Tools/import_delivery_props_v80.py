"""Import the original V80 OBJ handcart/crate as real UE5.6 static meshes.

Outside Unreal use --preflight; this checks source files and cannot create .uassets.
Generated assets live only in /Game/Askir/V80/Props. OBJ metres become centimetres.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import sys

NAMES = ('SM_AskirDeliveryCart', 'SM_AskirSupplyCrate')
DESTINATION = '/Game/Askir/V80/Props'
SOURCE_TAG = 'Askir.OriginalOBJ.SHA256'
MATERIAL_TAG = 'Askir.OriginalMTL.SHA256'


def source_preflight(project_root):
    """Stdlib-only preflight works inside Unreal's bundled Python without NumPy.

    Full OBJ/GLB triangle equivalence is checked separately by the offline verifier.
    """
    folder = Path(project_root)/'ArtSource/Delivery_V80'
    provenance = json.loads((folder/'PROVENANCE.json').read_text(encoding='utf-8'))
    expected = {name+suffix for name in NAMES for suffix in ('.blend', '.glb', '.obj', '.mtl', '_Preview.png')} | {'LICENSE_CC0.txt'}
    if (set(provenance.get('files', {})) != expected or set(provenance.get('props', {})) != set(NAMES)
            or provenance.get('license') != 'CC0-1.0' or provenance.get('unreal_obj_uniform_scale') != 100):
        raise ValueError('Invalid original prop provenance inventory, license or metre scale')
    for name, record in provenance['files'].items():
        raw = (folder/name).read_bytes()
        if len(raw) != record['bytes'] or hashlib.sha256(raw).hexdigest() != record['sha256']:
            raise ValueError('Original prop source checksum mismatch: '+name)
    for name in NAMES:
        vertices, faces = [], 0
        for line in (folder/(name+'.obj')).read_text(encoding='utf-8').splitlines():
            fields = line.split()
            if fields and fields[0] == 'v':
                point = [float(value) for value in fields[1:]]
                if len(point) != 3 or not all(math.isfinite(value) for value in point):
                    raise ValueError('Non-finite/invalid original OBJ coordinates')
                vertices.append(point)
            if fields and fields[0] == 'f':
                if len(fields) != 4:
                    raise ValueError('Original OBJ must contain actual triangles')
                faces += 1
        record = provenance['props'][name]
        if len(vertices) != record['vertices'] or faces != record['triangles'] or not vertices:
            raise ValueError('Original OBJ/provenance geometry counts mismatch')
        actual = [[min(v[i] for v in vertices) for i in range(3)], [max(v[i] for v in vertices) for i in range(3)]]
        if any(not math.isfinite(float(b)) or abs(a-b) > 2e-6 for x, y in zip(actual, record['bounds_metres_z_up']) for a, b in zip(x, y)):
            raise ValueError('Original OBJ/provenance bounds mismatch')
        raw = (folder/(name+'.glb')).read_bytes()
        if len(raw) < 128 or struct.unpack_from('<4sII', raw) != (b'glTF', 2, len(raw)):
            raise ValueError('Invalid actual GLB source header')
    return {'status': 'Bestanden', 'coverage': 'All source SHA256/byte sizes; actual OBJ counts, finite coordinates/bounds; GLB headers. Full triangle equivalence is verified offline.',
            'props': provenance['props'], 'files': provenance['files'],
            'unreal_import': 'Nicht ausgeführt', 'runtime_collision': 'Nicht ausgeführt'}


def _package_file(project_root, path):
    if path not in {DESTINATION+'/'+name for name in NAMES}:
        raise ValueError('Refusing unmanaged prop asset path: '+path)
    file = Path(project_root)/'Content'/(path[len('/Game/'):]+'.uasset')
    if not file.is_file() or file.stat().st_size < 128:
        raise RuntimeError('Editor reported save but actual .uasset missing/empty: '+str(file))
    return str(file)


def _original_materials(unreal, root, name):
    """Build explicit original PBR values; OBJ/MTL importers may ignore Pr/Pm."""
    records, current = {}, None
    for line in (root/'ArtSource/Delivery_V80'/(name+'.mtl')).read_text(encoding='utf-8').splitlines():
        fields = line.split()
        if fields and fields[0] == 'newmtl':
            current = fields[1]
            records[current] = {}
        elif fields and fields[0] in ('Kd', 'Pr', 'Pm'):
            records[current][fields[0]] = [float(value) for value in fields[1:]]
    result = {}
    library, editing = unreal.EditorAssetLibrary, unreal.MaterialEditingLibrary
    for material_name, values in records.items():
        path = DESTINATION+'/Materials/M_'+material_name
        material = unreal.load_asset(path) if library.does_asset_exist(path) else None
        if material is None:
            material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                'M_'+material_name, DESTINATION+'/Materials', unreal.Material, unreal.MaterialFactoryNew())
        if not isinstance(material, unreal.Material):
            raise RuntimeError('Original prop material has unexpected asset type: '+path)
        editing.delete_all_material_expressions(material)
        color = editing.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -400, 0)
        color.set_editor_property('constant', unreal.LinearColor(*values['Kd'], 1.0))
        if not editing.connect_material_property(color, '', unreal.MaterialProperty.MP_BASE_COLOR):
            raise RuntimeError('Cannot connect original prop base color: '+path)
        for offset, key, property in ((150, 'Pr', unreal.MaterialProperty.MP_ROUGHNESS),
                                      (300, 'Pm', unreal.MaterialProperty.MP_METALLIC)):
            expression = editing.create_material_expression(material, unreal.MaterialExpressionConstant, -400, offset)
            expression.set_editor_property('r', values[key][0])
            if not editing.connect_material_property(expression, '', property):
                raise RuntimeError('Cannot connect original prop PBR surface: '+path)
        editing.recompile_material(material)
        if not library.save_loaded_asset(material, only_if_is_dirty=False):
            raise RuntimeError('Cannot save original PBR prop material: '+path)
        actual_file = root/'Content'/(path[len('/Game/'):]+'.uasset')
        if not actual_file.is_file() or actual_file.stat().st_size < 128:
            raise RuntimeError('Actual original material package missing: '+path)
        result[material_name] = material
    return result


def _validate_mesh(unreal, root, name, source):
    path = DESTINATION+'/'+name
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError('Required imported prop is not a StaticMesh: '+path)
    actual_hash = unreal.EditorAssetLibrary.get_metadata_tag(mesh, SOURCE_TAG)
    expected_hash = source['files'][name+'.obj']['sha256']
    if str(actual_hash) != expected_hash:
        raise RuntimeError('Imported prop source identity does not match current original OBJ: '+path)
    if str(unreal.EditorAssetLibrary.get_metadata_tag(mesh, MATERIAL_TAG)) != source['files'][name+'.mtl']['sha256']:
        raise RuntimeError('Imported prop material identity does not match current original MTL: '+path)
    box = mesh.get_bounding_box()
    actual = [[float(box.min.x), float(box.min.y), float(box.min.z)],
              [float(box.max.x), float(box.max.y), float(box.max.z)]]
    expected = source['props'][name]['bounds_metres_z_up']
    if any(not math.isfinite(a) or abs(a-b*100) > .1 for x, y in zip(actual, expected) for a, b in zip(x, y)):
        raise RuntimeError('Imported metre/centimetre scale or Z-up orientation incorrect: '+path)
    materials = mesh.get_editor_property('static_materials')
    if len(materials) != source['props'][name]['materials'] or any(value.get_editor_property('material_interface') is None for value in materials):
        raise RuntimeError('Imported prop has missing original material slots: '+path)
    if mesh.get_num_sections(0) < 1:
        raise RuntimeError('Imported prop has no actual LOD0 mesh sections: '+path)
    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    collision_count = subsystem.get_simple_collision_count(mesh)
    if collision_count <= 0:
        raise RuntimeError('Imported prop lacks simple collision: '+path)
    return {'asset': path, 'package': _package_file(root, path), 'bounds_centimetres': actual,
            'simple_collision_primitives': int(collision_count), 'source_sha256': expected_hash}


def validate_imported_props(unreal, project_root):
    """Read-only asset validation for both generated and --use-existing-map flows."""
    source = source_preflight(project_root)
    records = {name: _validate_mesh(unreal, project_root, name, source) for name in NAMES}
    return {'status': 'Bestanden', 'assets': {name: item['asset'] for name, item in records.items()},
            'packages': [item['package'] for item in records.values()], 'props': records,
            'source_validation': source,
            'coverage': 'Real StaticMesh type, source identity, actual saved packages, scale/orientation/material slots, LOD0 sections and simple collision presence',
            'runtime_collision': 'Nicht ausgeführt', 'manual_appearance': 'Nicht ausgeführt'}


def import_delivery_props(unreal=None, project_root=None):
    """Import the two managed props and return status, assets and real package paths."""
    if unreal is None:
        import unreal as unreal_module
        unreal = unreal_module
    root = Path(project_root) if project_root is not None else Path(__file__).resolve().parents[1]
    version = unreal.SystemLibrary.get_engine_version()
    if not version.startswith('5.6.'):
        raise RuntimeError('Unreal Engine 5.6 required, got '+version)
    source = source_preflight(root)  # Validate every input before engine mutation.
    library = unreal.EditorAssetLibrary
    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    for name in NAMES:
        path = DESTINATION+'/'+name
        if library.does_asset_exist(path) and not isinstance(unreal.load_asset(path), unreal.StaticMesh):
            raise RuntimeError('Refusing to replace non-StaticMesh asset: '+path)
        # This namespace contains these original generated meshes only. Reimport
        # deliberately updates changed sources instead of silently keeping stale geometry.
        task = unreal.AssetImportTask()
        for key, value in {'filename': str(root/'ArtSource/Delivery_V80'/(name+'.obj')),
                           'destination_path': DESTINATION, 'destination_name': name,
                           'automated': True, 'replace_existing': True, 'replace_existing_settings': True,
                           'save': True}.items():
            task.set_editor_property(key, value)
        options = unreal.FbxImportUI()
        for key, value in {'automated_import_should_detect_type': False, 'import_as_skeletal': False,
                           'mesh_type_to_import': unreal.FBXImportType.FBXIT_STATIC_MESH,
                           'import_mesh': True, 'import_materials': True, 'import_textures': False,
                           'import_animations': False}.items():
            options.set_editor_property(key, value)
        data = options.get_editor_property('static_mesh_import_data')
        for key, value in {'combine_meshes': True, 'auto_generate_collision': False,
                           'generate_lightmap_u_vs': True, 'import_uniform_scale': 100.0,
                           'convert_scene': False, 'convert_scene_unit': False,
                           'force_front_x_axis': False}.items():
            data.set_editor_property(key, value)
        task.set_editor_property('options', options)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        imported_paths = list(task.get_editor_property('imported_object_paths'))
        mesh = unreal.load_asset(path)
        if not isinstance(mesh, unreal.StaticMesh) or not any(value.split('.', 1)[0] == path for value in imported_paths):
            raise RuntimeError('OBJ import did not return the required actual StaticMesh: '+path)
        originals = _original_materials(unreal, root, name)
        for slot_index, slot in enumerate(mesh.get_editor_property('static_materials')):
            original_name = str(slot.get_editor_property('imported_material_slot_name'))
            if original_name not in originals:
                original_name = str(slot.get_editor_property('material_slot_name'))
            if original_name not in originals:
                raise RuntimeError('OBJ importer lost original named material slot: '+path)
            mesh.set_material(slot_index, originals[original_name])
        subsystem.remove_collisions(mesh)
        index = subsystem.add_simple_collisions(mesh, unreal.ScriptingCollisionShapeType.BOX)
        if index < 0:
            raise RuntimeError('Cannot generate required simple box collision: '+path)
        library.set_metadata_tag(mesh, SOURCE_TAG, source['files'][name+'.obj']['sha256'])
        library.set_metadata_tag(mesh, MATERIAL_TAG, source['files'][name+'.mtl']['sha256'])
        if not library.save_loaded_asset(mesh, only_if_is_dirty=False):
            raise RuntimeError('Required static mesh save failed: '+path)
        for slot in mesh.get_editor_property('static_materials'):
            mat = slot.get_editor_property('material_interface')
            if mat is None or not library.save_loaded_asset(mat, only_if_is_dirty=False):
                raise RuntimeError('Original imported prop material could not be saved: '+path)
        _validate_mesh(unreal, root, name, source)
    result = validate_imported_props(unreal, root)
    result['engine_version'] = version
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--preflight', action='store_true')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args(argv)
    report = args.report or args.project_root/'Reports'/('V80_PROP_SOURCE_PREFLIGHT.json' if args.preflight else 'V80_PROP_IMPORT.json')
    code = 0
    try:
        if args.preflight:
            result = source_preflight(args.project_root)
        else:
            try:
                import unreal
            except ImportError:
                result, code = {'status': 'Blockiert', 'error': 'Unreal Engine 5.6 Python API unavailable', 'runtime_collision': 'Nicht ausgeführt'}, 2
            else:
                result = import_delivery_props(unreal, args.project_root)
    except Exception as exc:
        result, code = {'status': 'Fehlgeschlagen', 'error': str(exc), 'runtime_collision': 'Nicht ausgeführt'}, 1
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(result, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print('ASKIR V80 delivery prop import:', result['status'], report)
    return code


if __name__ == '__main__':
    # UE owns sys.argv; source imports must never execute the commandlet implicitly.
    raise SystemExit(main([] if 'unreal' in sys.modules else None))
