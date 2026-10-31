"""Import original V84 healing herbs through the established metre/Z-up OBJ path.

FBX, GLB and Blender are retained original sources; --preflight checks sources only.
Real imported assets and packages are verified exclusively inside Unreal5.6.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE_NAME = 'Askir_HealingHerbs'
NAMES = ('SM_AskirHealingHerbs',)
DESTINATION = '/Game/Askir/V84/Props'
SOURCE_TAG = 'Askir.OriginalOBJ.SHA256'
MATERIAL_TAG = 'Askir.OriginalMTL.SHA256'


def _finite_vector(values, length, label):
    if not isinstance(values, list) or len(values) != length or any(type(value) not in (int, float) or not math.isfinite(value) for value in values):
        raise ValueError('Invalid finite '+label)
    return values


def _read_original_geometry(folder, original_materials):
    """Reject malformed/external OBJ/MTL data before any editor asset mutation."""
    materials, current = {}, None
    for line in (folder/(SOURCE_NAME+'.mtl')).read_text(encoding='utf-8').splitlines():
        fields = line.split()
        if not fields or fields[0].startswith('#'):
            continue
        if fields[0].lower().startswith('map_'):
            raise ValueError('External herb material textures are forbidden')
        if fields[0] == 'newmtl':
            if len(fields) != 2 or fields[1] in materials:
                raise ValueError('Invalid or duplicate original herb material')
            current = fields[1]
            materials[current] = {}
        elif fields[0] in ('Kd', 'Pr', 'Pm'):
            if current is None or fields[0] in materials[current]:
                raise ValueError('Herb PBR material values precede a material or repeat')
            values = _finite_vector([float(value) for value in fields[1:]], 3 if fields[0] == 'Kd' else 1, 'herb PBR values')
            if any(value < 0 or value > 1 for value in values):
                raise ValueError('Herb PBR material values are out of range')
            materials[current][fields[0]] = values
    if set(materials) != set(original_materials) or any(set(values) != {'Kd', 'Pr', 'Pm'} for values in materials.values()):
        raise ValueError('Incomplete or unexpected original herb material inventory')
    for name, values in materials.items():
        original = original_materials[name]
        for actual, expected in zip(values['Kd']+values['Pr']+values['Pm'], original['base_color'][:3]+[original['roughness'], original['metallic']]):
            if abs(actual-expected) > 1e-5:
                raise ValueError('Herb MTL/PBR metadata values differ')
    vertices, normals, faces, material, libraries = [], [], 0, None, 0
    for line in (folder/(SOURCE_NAME+'.obj')).read_text(encoding='utf-8').splitlines():
        fields = line.split()
        if not fields or fields[0].startswith('#'):
            continue
        if fields[0] == 'mtllib':
            libraries += 1
            if libraries != 1 or fields[1:] != [SOURCE_NAME+'.mtl']:
                raise ValueError('Unsafe or unexpected herb OBJ material path')
        elif fields[0] in ('v', 'vn'):
            values = _finite_vector([float(value) for value in fields[1:]], 3, 'original herb OBJ coordinates/normals')
            if fields[0] == 'vn':
                if abs(sum(value*value for value in values)-1) > .0001:
                    raise ValueError('Non-unit herb OBJ normal')
                normals.append(values)
            else:
                vertices.append(values)
        elif fields[0] == 'usemtl':
            if len(fields) != 2 or fields[1] not in materials:
                raise ValueError('Unknown original herb OBJ material')
            material = fields[1]
        elif fields[0] == 'f':
            if len(fields) != 4 or material is None:
                raise ValueError('Herb OBJ must contain material-assigned triangles')
            indices = []
            for field in fields[1:]:
                parts = field.split('//')
                if len(parts) != 2:
                    raise ValueError('Invalid herb OBJ face format')
                vertex, normal = map(int, parts)
                if not 0 < vertex <= len(vertices) or not 0 < normal <= len(normals):
                    raise ValueError('Herb OBJ triangle index out of range')
                indices.append(vertex-1)
            a, b, c = (vertices[index] for index in indices)
            u, v = [b[i]-a[i] for i in range(3)], [c[i]-a[i] for i in range(3)]
            cross = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
            if len(set(indices)) != 3 or sum(value*value for value in cross) <= 1e-20:
                raise ValueError('Degenerate herb OBJ triangle')
            faces += 1
    if libraries != 1:
        raise ValueError('Missing managed herb OBJ material library')
    return vertices, faces


def source_preflight(project_root=ROOT):
    """Stdlib checks run in embedded Python without starting another process."""
    root = Path(project_root)
    folder = root/'ArtSource/Environment_V84'
    source = json.loads((root/'Production/Props/herb_assets_v84.json').read_text(encoding='utf-8'))
    expected = {SOURCE_NAME+suffix for suffix in ('.blend', '.fbx', '.glb', '.obj', '.mtl', '_Preview.png')} | {'LICENSE_CC0.txt'}
    if (source.get('version') != 'V84' or source.get('license') != 'CC0-1.0' or
            source.get('unreal_obj_uniform_scale') != 100 or source.get('source_folder') != 'ArtSource/Environment_V84' or
            source.get('source_name') != SOURCE_NAME or source.get('mesh_name') != NAMES[0] or
            source.get('asset') != DESTINATION+'/'+NAMES[0] or set(source.get('files', {})) != expected):
        raise ValueError('Invalid healing herb inventory, path, license or metre scale')
    for name, record in source['files'].items():
        raw = (folder/name).read_bytes()
        if type(record.get('bytes')) is not int or len(raw) != record['bytes'] or hashlib.sha256(raw).hexdigest() != record.get('sha256'):
            raise ValueError('Healing herb source checksum mismatch: '+name)
    fbx = (folder/(SOURCE_NAME+'.fbx')).read_bytes()
    if len(fbx) < 1024 or not fbx.startswith(b'Kaydara FBX Binary  \x00\x1a\x00') or struct.unpack_from('<I', fbx, 23)[0] not in (7400, 7500):
        raise ValueError('Invalid actual binary FBX source header')
    if not (folder/(SOURCE_NAME+'.blend')).read_bytes().startswith(b'BLENDER'):
        raise ValueError('Invalid original Blender source header')
    png = (folder/(SOURCE_NAME+'_Preview.png')).read_bytes()
    if png[:8] != b'\x89PNG\r\n\x1a\n' or len(png) < 32 or struct.unpack_from('>II', png, 16) != (960, 720):
        raise ValueError('Invalid actual Blender preview header')
    raw = (folder/(SOURCE_NAME+'.glb')).read_bytes()
    if len(raw) < 128 or struct.unpack_from('<4sII', raw) != (b'glTF', 2, len(raw)):
        raise ValueError('Invalid actual GLB source header')
    record = source['geometry']
    if any(type(record.get(key)) is not int or record[key] <= 0 for key in ('vertices', 'triangles', 'materials')):
        raise ValueError('Invalid herb geometry count metadata')
    bounds = record.get('bounds_metres_z_up')
    if not isinstance(bounds, list) or len(bounds) != 2:
        raise ValueError('Invalid herb provenance bounds')
    for bound in bounds:
        _finite_vector(bound, 3, 'herb provenance bounds')
    _finite_vector(record.get('dimensions_centimetres'), 3, 'herb provenance dimensions')
    if not isinstance(source.get('materials'), dict) or len(source['materials']) != record['materials'] or not all(re.fullmatch(r'Askir_HealingHerbs_[A-Za-z]+', name) for name in source['materials']):
        raise ValueError('Invalid herb material namespace or inventory')
    for material in source['materials'].values():
        color = _finite_vector(material.get('base_color'), 4, 'herb material color')
        values = color + _finite_vector([material.get('roughness'), material.get('metallic')], 2, 'herb material surface')
        if any(value < 0 or value > 1 for value in values):
            raise ValueError('Invalid original herb PBR material values')
    vertices, faces = _read_original_geometry(folder, source['materials'])
    if len(vertices) != record['vertices'] or faces != record['triangles'] or not vertices:
        raise ValueError('Herb OBJ/provenance geometry counts mismatch')
    actual = [[min(v[i] for v in vertices) for i in range(3)], [max(v[i] for v in vertices) for i in range(3)]]
    if any(not math.isfinite(float(b)) or abs(a-b) > 2e-6 for x, y in zip(actual, record['bounds_metres_z_up']) for a, b in zip(x, y)):
        raise ValueError('Herb OBJ/provenance bounds mismatch')
    dimensions = [(actual[1][axis]-actual[0][axis])*100 for axis in range(3)]
    if (not 50 <= max(dimensions[:2]) <= 100 or not 50 <= dimensions[2] <= 80 or abs(actual[0][2]) > .0001 or
            any(not math.isfinite(float(value)) or abs(value-dimensions[index]) > .001 for index, value in enumerate(record['dimensions_centimetres']))):
        raise ValueError('Herb source must have floor-level feet and authored centimetre bounds')
    return {'status': 'Bestanden', 'coverage': 'All source SHA256/byte sizes; actual OBJ indices, nondegenerate triangles, unit normals, safe managed MTL and PBR values, counts, finite coordinates/bounds; GLB/FBX/Blender/preview headers. Cross-format GLB/OBJ topology requires the full offline validator.',
            'props': {NAMES[0]: record}, 'materials': source['materials'], 'files': source['files'],
            'asset': source['asset'], 'editor_status': 'Nicht ausgeführt',
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
    for line in (root/'ArtSource/Environment_V84'/(SOURCE_NAME+'.mtl')).read_text(encoding='utf-8').splitlines():
        fields = line.split()
        if fields and fields[0] == 'newmtl':
            current = fields[1]
            records[current] = {}
        elif fields and fields[0] in ('Kd', 'Pr', 'Pm'):
            records[current][fields[0]] = [float(value) for value in fields[1:]]
    source = source_preflight(root)
    if set(records) != set(source['materials']):
        raise RuntimeError('Herb MTL/material inventory mismatch before import')
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
        library.set_metadata_tag(material, MATERIAL_TAG, source['files'][SOURCE_NAME+'.mtl']['sha256'])
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
    expected_hash = source['files'][SOURCE_NAME+'.obj']['sha256']
    if str(actual_hash) != expected_hash:
        raise RuntimeError('Imported prop source identity does not match current original OBJ: '+path)
    if str(unreal.EditorAssetLibrary.get_metadata_tag(mesh, MATERIAL_TAG)) != source['files'][SOURCE_NAME+'.mtl']['sha256']:
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
    material_records = []
    seen = set()
    for slot in materials:
        mat = slot.get_editor_property('material_interface')
        if not isinstance(mat, unreal.Material):
            raise RuntimeError('Herb material slot is not an original Material asset')
        material_name = mat.get_name()
        if not material_name.startswith('M_') or material_name[2:] not in source['materials'] or material_name in seen:
            raise RuntimeError('Herb mesh has missing, duplicate or unexpected original material slots')
        seen.add(material_name)
        material_path = DESTINATION+'/Materials/'+material_name
        if mat.get_path_name().split('.', 1)[0] != material_path:
            raise RuntimeError('Herb material is outside the managed original asset namespace')
        if str(unreal.EditorAssetLibrary.get_metadata_tag(mat, MATERIAL_TAG)) != source['files'][SOURCE_NAME+'.mtl']['sha256']:
            raise RuntimeError('Saved herb material source identity is stale')
        actual_file = Path(root)/'Content'/(material_path[len('/Game/'):]+'.uasset')
        if not actual_file.is_file() or actual_file.stat().st_size < 128:
            raise RuntimeError('Actual saved herb material package missing')
        expected_material = source['materials'][material_name[2:]]
        for property, kind, field, expected_values in (
                (unreal.MaterialProperty.MP_BASE_COLOR, unreal.MaterialExpressionConstant3Vector, 'constant', expected_material['base_color'][:3]),
                (unreal.MaterialProperty.MP_ROUGHNESS, unreal.MaterialExpressionConstant, 'r', [expected_material['roughness']]),
                (unreal.MaterialProperty.MP_METALLIC, unreal.MaterialExpressionConstant, 'r', [expected_material['metallic']])):
            node = unreal.MaterialEditingLibrary.get_material_property_input_node(mat, property)
            if not isinstance(node, kind):
                raise RuntimeError('Saved herb material no longer has its original PBR constant graph')
            value = node.get_editor_property(field)
            actual_values = [value.r, value.g, value.b] if field == 'constant' else [value]
            if any(not math.isfinite(float(a)) or abs(a-b) > 1e-5 for a, b in zip(actual_values, expected_values)):
                raise RuntimeError('Saved herb material PBR values differ from original source')
        material_records.append({'asset': material_path, 'package': str(actual_file), 'original_pbr': 'Bestanden'})
    return {'asset': path, 'package': _package_file(root, path), 'bounds_centimetres': actual,
            'materials': material_records, 'simple_collision_primitives': int(collision_count), 'source_sha256': expected_hash}


def validate_imported_assets(unreal, project_root=ROOT):
    """Read-only asset validation for both generated and --use-existing-map flows."""
    source = source_preflight(project_root)
    records = {name: _validate_mesh(unreal, project_root, name, source) for name in NAMES}
    return {'status': 'Bestanden', 'editor_status': 'Bestanden', 'assets': {name: item['asset'] for name, item in records.items()},
            'packages': [item['package'] for item in records.values()], 'props': records,
            'source_validation': source,
            'coverage': 'Real StaticMesh type, source identity, actual saved packages, scale/orientation/material slots, LOD0 sections and simple collision presence',
            'runtime_collision': 'Nicht ausgeführt', 'manual_appearance': 'Nicht ausgeführt'}


def reload_saved_assets(unreal, root, package_paths):
    """Explicit package reload; a cached load_asset call is not reload evidence."""
    loader = getattr(unreal, 'load_package', None)
    utils = getattr(unreal, 'EditorLoadingAndSavingUtils', None)
    reloader = getattr(utils, 'reload_packages', None)
    modes = getattr(unreal, 'ReloadPackagesInteractionMode', None)
    mode = getattr(modes, 'ASSUME_POSITIVE', None)
    if not callable(loader) or not callable(reloader) or mode is None:
        raise RuntimeError('UE5.6 explicit package reload API unavailable; cannot validate saved herb reload')
    packages = []
    for path in package_paths:
        if not path.startswith(DESTINATION+'/') or '..' in path or path.endswith('.'):
            raise ValueError('Refusing unmanaged herb package reload')
        file = Path(root)/'Content'/(path[len('/Game/'):]+'.uasset')
        if not file.is_file() or file.stat().st_size < 128:
            raise RuntimeError('Cannot reload an absent herb package')
        package = loader(None, path)
        if package is None:
            raise RuntimeError('Could not load saved herb package for reload: '+path)
        packages.append(package)
    if not packages:
        raise RuntimeError('Cannot prove reload of an empty herb package list')
    result = reloader(packages, interaction_mode=mode)
    # Python exposes UE's bool + OutErrorMessage either as a tuple or a bool.
    success = result[0] if isinstance(result, tuple) and result else result
    if success is not True:
        raise RuntimeError('Explicit saved herb package reload failed')
    return {'status': 'Bestanden', 'packages': list(package_paths), 'method': 'EditorLoadingAndSavingUtils.reload_packages'}


def import_assets(unreal=None, root=ROOT):
    """Import the managed herb mesh and return evidence from actual saved packages."""
    if unreal is None:
        import unreal as unreal_module
        unreal = unreal_module
    root = Path(root)
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
        for key, value in {'filename': str(root/'ArtSource/Environment_V84'/(SOURCE_NAME+'.obj')),
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
        library.set_metadata_tag(mesh, SOURCE_TAG, source['files'][SOURCE_NAME+'.obj']['sha256'])
        library.set_metadata_tag(mesh, MATERIAL_TAG, source['files'][SOURCE_NAME+'.mtl']['sha256'])
        if not library.save_loaded_asset(mesh, only_if_is_dirty=False):
            raise RuntimeError('Required static mesh save failed: '+path)
        for slot in mesh.get_editor_property('static_materials'):
            mat = slot.get_editor_property('material_interface')
            if mat is None or not library.save_loaded_asset(mat, only_if_is_dirty=False):
                raise RuntimeError('Original imported prop material could not be saved: '+path)
        _validate_mesh(unreal, root, name, source)
    package_paths = [DESTINATION+'/'+NAMES[0]] + [DESTINATION+'/Materials/M_'+name for name in source['materials']]
    reloaded = reload_saved_assets(unreal, root, package_paths)
    result = validate_imported_assets(unreal, root)
    result['package_reload'] = reloaded
    result['engine_version'] = version
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--preflight', action='store_true')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args(argv)
    report = args.report or args.project_root/'Reports'/('V84_HERB_SOURCE_PREFLIGHT.json' if args.preflight else 'V84_HERB_IMPORT.json')
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
                result = import_assets(unreal, args.project_root)
    except Exception as exc:
        result, code = {'status': 'Fehlgeschlagen', 'error': str(exc), 'runtime_collision': 'Nicht ausgeführt'}, 1
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(result, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print('ASKIR V84 herb asset import:', result['status'], report)
    return code


if __name__ == '__main__':
    # UE owns sys.argv; source imports must never execute the commandlet implicitly.
    raise SystemExit(main([] if 'unreal' in sys.modules else None))
