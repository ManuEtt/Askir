"""Original source topology and editor-import failure boundaries, never engine evidence."""
from __future__ import annotations

from contextlib import contextmanager
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import struct
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
FOLDER = ROOT/'ArtSource/Environment_V84'
NAME = 'Askir_HealingHerbs'


def module(name, filename):
    spec = importlib.util.spec_from_file_location(name, ROOT/'Tools'/filename)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


IMPORTER = module('_herb_asset_import_tests', 'import_herb_assets_v84.py')
VALIDATOR = module('_herb_asset_validation_tests', 'validate_herb_assets_v84.py')
GEOMETRY = module('_herb_actual_binary_geometry_tests', 'validate_delivery_props_v80.py')


class OriginalHerbSourceTests(unittest.TestCase):
    @contextmanager
    def project(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            shutil.copytree(FOLDER, root/'ArtSource/Environment_V84')
            (root/'Production/Props').mkdir(parents=True)
            shutil.copyfile(ROOT/'Production/Props/herb_assets_v84.json', root/'Production/Props/herb_assets_v84.json')
            (root/'Tools').mkdir()
            for name in ('import_herb_assets_v84.py', 'validate_delivery_props_v80.py'):
                shutil.copyfile(ROOT/'Tools'/name, root/'Tools'/name)
            yield root

    def metadata(self, root, change):
        path = root/'Production/Props/herb_assets_v84.json'
        value = json.loads(path.read_text())
        change(value)
        path.write_text(json.dumps(value))

    def changed_source(self, root, suffix, change):
        path = root/'ArtSource/Environment_V84'/(NAME+suffix)
        path.write_bytes(change(path.read_bytes()))
        self.metadata(root, lambda value: value['files'][path.name].update(
            bytes=path.stat().st_size, sha256=hashlib.sha256(path.read_bytes()).hexdigest()))

    def replace_line(self, raw, prefix, replacement):
        lines = raw.decode().splitlines()
        index = next(index for index, line in enumerate(lines) if line.startswith(prefix))
        lines[index] = replacement
        return ('\n'.join(lines)+'\n').encode()

    def test_actual_source_buffers_and_metadata_agree(self):
        result = VALIDATOR.validate(ROOT)
        self.assertEqual(result['status'], 'Bestanden')
        self.assertEqual(result['geometry']['triangles'], 1848)
        self.assertEqual(result['geometry']['materials'], 6)
        self.assertEqual(result['geometry']['glb_obj_triangle_geometry'], 'Bestanden')
        self.assertEqual(result['unreal_import'], 'Nicht ausgeführt')
        self.assertEqual(result['runtime_collision'], 'Nicht ausgeführt')

    def test_editor_preflight_is_stdlib_only_and_starts_no_process(self):
        with patch.dict(sys.modules, {'numpy': None}), patch('subprocess.run', side_effect=AssertionError('No editor subprocess')):
            result = IMPORTER.source_preflight(ROOT)
        self.assertEqual(result['asset'], '/Game/Askir/V84/Props/SM_AskirHealingHerbs')
        self.assertEqual(result['editor_status'], 'Nicht ausgeführt')

    def test_actual_file_checksum_change_is_rejected(self):
        with self.project() as root:
            path = root/'ArtSource/Environment_V84'/(NAME+'.obj')
            path.write_bytes(path.read_bytes()+b'# changed source\n')
            with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
                IMPORTER.source_preflight(root)

    def test_rehashed_external_obj_material_path_is_rejected_before_import(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: raw.replace(b'mtllib Askir_HealingHerbs.mtl', b'mtllib ../../outside.mtl'))
            with self.assertRaisesRegex(ValueError, 'material path'):
                IMPORTER.source_preflight(root)

    def test_rehashed_out_of_range_vertex_index_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: self.replace_line(raw, 'f ', 'f 999999//1 2//1 3//1'))
            with self.assertRaisesRegex(ValueError, 'index out of range'):
                IMPORTER.source_preflight(root)

    def test_rehashed_out_of_range_normal_index_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: self.replace_line(raw, 'f ', 'f 1//999999 2//1 3//1'))
            with self.assertRaisesRegex(ValueError, 'index out of range'):
                IMPORTER.source_preflight(root)

    def test_rehashed_degenerate_triangle_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: self.replace_line(raw, 'f ', 'f 1//1 1//1 3//1'))
            with self.assertRaisesRegex(ValueError, 'Degenerate'):
                IMPORTER.source_preflight(root)

    def test_rehashed_nonfinite_vertex_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: self.replace_line(raw, 'v ', 'v nan 0 0'))
            with self.assertRaisesRegex(ValueError, 'finite'):
                IMPORTER.source_preflight(root)

    def test_rehashed_nonunit_normal_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: self.replace_line(raw, 'vn ', 'vn 0 0 0'))
            with self.assertRaisesRegex(ValueError, 'Non-unit'):
                IMPORTER.source_preflight(root)

    def test_rehashed_unknown_obj_material_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.obj', lambda raw: self.replace_line(raw, 'usemtl ', 'usemtl ForeignMaterial'))
            with self.assertRaisesRegex(ValueError, 'Unknown original'):
                IMPORTER.source_preflight(root)

    def test_rehashed_mtl_values_must_match_original_metadata(self):
        with self.project() as root:
            self.changed_source(root, '.mtl', lambda raw: self.replace_line(raw, 'Kd ', 'Kd 0.99 0.99 0.99'))
            with self.assertRaisesRegex(ValueError, 'metadata values differ'):
                IMPORTER.source_preflight(root)

    def test_rehashed_external_texture_path_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.mtl', lambda raw: raw+b'\nmap_Kd ../../external.png\n')
            with self.assertRaisesRegex(ValueError, 'External herb material textures'):
                IMPORTER.source_preflight(root)

    def test_rehashed_duplicate_pbr_values_are_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.mtl', lambda raw: raw.replace(b'Pr 0.95000', b'Pr 0.95000\nPr 0.95000'))
            with self.assertRaisesRegex(ValueError, 'repeat'):
                IMPORTER.source_preflight(root)

    def test_invalid_scale_or_license_or_managed_asset_path_is_rejected(self):
        for key, wrong in (('unreal_obj_uniform_scale', 1), ('license', 'Unknown'), ('asset', '/Game/Foreign/Replacement')):
            with self.subTest(field=key), self.project() as root:
                self.metadata(root, lambda value: value.update({key: wrong}))
                with self.assertRaisesRegex(ValueError, 'inventory, path, license or metre scale'):
                    IMPORTER.source_preflight(root)

    def test_partial_bounds_and_boolean_geometry_counts_cannot_hide_in_zip_comparison(self):
        for key, wrong in (('vertices', True), ('bounds_metres_z_up', [[0, 0], [1, 1]]), ('dimensions_centimetres', [58, 57])):
            with self.subTest(field=key), self.project() as root:
                self.metadata(root, lambda value: value['geometry'].update({key: wrong}))
                with self.assertRaises(ValueError):
                    IMPORTER.source_preflight(root)

    def test_rehashed_fake_fbx_header_is_rejected(self):
        with self.project() as root:
            self.changed_source(root, '.fbx', lambda raw: b'Not an FBX source'.ljust(23, b' ')+raw[23:])
            with self.assertRaisesRegex(ValueError, 'FBX source header'):
                IMPORTER.source_preflight(root)

    def corrupt_glb_accessor(self, root, kind, format, value):
        path = root/'ArtSource/Environment_V84'/(NAME+'.glb')
        glb = GEOMETRY.GLB(path)
        primitive = glb.data['meshes'][0]['primitives'][0]
        index = primitive['indices'] if kind == 'indices' else primitive['attributes'][kind]
        accessor = glb.data['accessors'][index]
        view = glb.data['bufferViews'][accessor['bufferView']]
        def mutate(raw):
            binary_start = 28+struct.unpack_from('<I', raw, 12)[0]
            changed = bytearray(raw)
            struct.pack_into(format, changed, binary_start+view.get('byteOffset', 0)+accessor.get('byteOffset', 0), value)
            return bytes(changed)
        self.changed_source(root, '.glb', mutate)

    def test_rehashed_nan_glb_position_is_rejected_by_full_buffer_validation(self):
        with self.project() as root:
            self.corrupt_glb_accessor(root, 'POSITION', '<f', float('nan'))
            with self.assertRaisesRegex(ValueError, 'Non-finite GLB positions'):
                VALIDATOR.validate(root)

    def test_rehashed_glb_nonfinite_normal_is_rejected_by_full_buffer_validation(self):
        with self.project() as root:
            self.corrupt_glb_accessor(root, 'NORMAL', '<f', float('inf'))
            with self.assertRaisesRegex(ValueError, 'GLB normals'):
                VALIDATOR.validate(root)


class HerbEditorImportEvidenceBoundaries(unittest.TestCase):
    def test_package_check_rejects_other_namespace_and_absent_package(self):
        with self.assertRaisesRegex(ValueError, 'unmanaged prop'):
            IMPORTER._package_file(ROOT, '/Game/Other/SM_AskirHealingHerbs')
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(RuntimeError, 'missing/empty'):
                IMPORTER._package_file(temporary, IMPORTER.DESTINATION+'/'+IMPORTER.NAMES[0])

    def test_unknown_editor_version_cannot_mutate_assets(self):
        fake = SimpleNamespace(SystemLibrary=SimpleNamespace(get_engine_version=lambda: '5.5.0'))
        with self.assertRaisesRegex(RuntimeError, '5.6 required'):
            IMPORTER.import_assets(fake, ROOT)

    def test_reload_requires_actual_api_and_real_packages(self):
        with self.assertRaisesRegex(RuntimeError, 'reload API unavailable'):
            IMPORTER.reload_saved_assets(SimpleNamespace(), ROOT, [IMPORTER.DESTINATION+'/'+IMPORTER.NAMES[0]])
        fake = SimpleNamespace(load_package=lambda outer, name: object(),
                               EditorLoadingAndSavingUtils=SimpleNamespace(reload_packages=lambda *args, **kwargs: True),
                               ReloadPackagesInteractionMode=SimpleNamespace(ASSUME_POSITIVE=1))
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(RuntimeError, 'absent herb package'):
                IMPORTER.reload_saved_assets(fake, temporary, [IMPORTER.DESTINATION+'/'+IMPORTER.NAMES[0]])

    def test_reload_refuses_other_namespace_and_false_editor_result(self):
        fake = SimpleNamespace(load_package=lambda outer, name: object(),
                               EditorLoadingAndSavingUtils=SimpleNamespace(reload_packages=lambda *args, **kwargs: (False, 'fixture error')),
                               ReloadPackagesInteractionMode=SimpleNamespace(ASSUME_POSITIVE=1))
        with self.assertRaisesRegex(ValueError, 'unmanaged herb package'):
            IMPORTER.reload_saved_assets(fake, ROOT, ['/Game/Outside/Anything'])
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)/'Content/Askir/V84/Props/SM_AskirHealingHerbs.uasset'
            path.parent.mkdir(parents=True)
            # This explicit invalid fixture proves only failure handling; it is
            # never packaged or counted as an actual Unreal asset.
            path.write_bytes(b'fixture, not an Unreal package'.ljust(128, b' '))
            with self.assertRaisesRegex(RuntimeError, 'package reload failed'):
                IMPORTER.reload_saved_assets(fake, temporary, [IMPORTER.DESTINATION+'/'+IMPORTER.NAMES[0]])


if __name__ == '__main__':
    unittest.main()
