"""Actual original delivery geometry and deliberate binary/source corruption tests."""
import copy
import importlib.util
import json
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
FOLDER = ROOT/'ArtSource/Delivery_V80'


def module(name, filename):
    spec = importlib.util.spec_from_file_location(name, ROOT/'Tools'/filename)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


VALIDATOR = module('askir_delivery_props_validation', 'validate_delivery_props_v80.py')
IMPORTER = module('askir_delivery_props_import', 'import_delivery_props_v80.py')


class DeliveryPropTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.metadata = json.loads((FOLDER/'PROVENANCE.json').read_text())

    def source(self):
        return VALIDATOR.GLB(FOLDER/(VALIDATOR.NAMES[0]+'.glb'))

    def validate_modified(self, glb):
        with patch.object(VALIDATOR, 'GLB', return_value=glb):
            return VALIDATOR.validate_prop(FOLDER, VALIDATOR.NAMES[0], self.metadata['props'][VALIDATOR.NAMES[0]])

    def overwrite(self, glb, index, format, value):
        access = glb.data['accessors'][index]
        view = glb.data['bufferViews'][access['bufferView']]
        raw = bytearray(glb.binary)
        struct.pack_into(format, raw, view.get('byteOffset', 0)+access.get('byteOffset', 0), value)
        glb.binary = bytes(raw)

    def test_actual_obj_glb_geometry_materials_and_all_source_hashes(self):
        report = VALIDATOR.validate(ROOT)
        self.assertEqual(report['status'], 'Bestanden')
        self.assertEqual(report['props']['SM_AskirDeliveryCart']['triangles'], 4532)
        self.assertEqual(report['props']['SM_AskirSupplyCrate']['triangles'], 3232)
        self.assertEqual(report['unreal_import'], 'Nicht ausgeführt')

    def test_unreal_stdlib_preflight_matches_full_source_inventory(self):
        result = IMPORTER.source_preflight(ROOT)
        self.assertEqual(result['status'], 'Bestanden')
        self.assertEqual(set(result['props']), set(VALIDATOR.NAMES))
        self.assertEqual(len(result['files']), 11)

    def test_actual_glb_position_nan_rejected(self):
        glb = self.source()
        index = glb.data['meshes'][0]['primitives'][0]['attributes']['POSITION']
        self.overwrite(glb, index, '<f', float('nan'))
        with self.assertRaisesRegex(ValueError, 'Non-finite GLB positions'):
            self.validate_modified(glb)

    def test_actual_glb_normal_nan_rejected(self):
        glb = self.source()
        index = glb.data['meshes'][0]['primitives'][0]['attributes']['NORMAL']
        self.overwrite(glb, index, '<f', float('nan'))
        with self.assertRaisesRegex(ValueError, 'GLB normals'):
            self.validate_modified(glb)

    def test_actual_glb_position_geometry_divergence_rejected(self):
        glb = self.source()
        index = glb.data['meshes'][0]['primitives'][0]['attributes']['POSITION']
        self.overwrite(glb, index, '<f', 90.0)
        with self.assertRaisesRegex(ValueError, 'geometry mismatch'):
            self.validate_modified(glb)

    def test_actual_glb_triangle_index_out_of_range_rejected(self):
        glb = self.source()
        index = glb.data['meshes'][0]['primitives'][0]['indices']
        access = glb.data['accessors'][index]
        self.overwrite(glb, index, '<H' if access['componentType'] == 5123 else '<I', 65535)
        with self.assertRaisesRegex(ValueError, 'triangle indices'):
            self.validate_modified(glb)

    def test_actual_glb_degenerate_triangle_rejected(self):
        glb = self.source()
        index = glb.data['meshes'][0]['primitives'][0]['indices']
        values = glb.accessor(index)
        access = glb.data['accessors'][index]
        view = glb.data['bufferViews'][access['bufferView']]
        fmt = '<H' if access['componentType'] == 5123 else '<I'
        width = struct.calcsize(fmt)
        raw = bytearray(glb.binary)
        offset = view.get('byteOffset', 0)+access.get('byteOffset', 0)
        struct.pack_into(fmt, raw, offset+width, int(values[0, 0]))
        glb.binary = bytes(raw)
        with self.assertRaisesRegex(ValueError, 'Degenerate GLB triangle'):
            self.validate_modified(glb)

    def test_material_color_mismatch_rejected(self):
        glb = self.source()
        glb.data['materials'][0]['pbrMetallicRoughness']['baseColorFactor'][0] = .99
        with self.assertRaisesRegex(ValueError, 'color mismatch'):
            self.validate_modified(glb)

    def test_material_topology_mismatch_rejected(self):
        glb = self.source()
        glb.data['meshes'][0]['primitives'][0]['material'] = 1
        with self.assertRaisesRegex(ValueError, 'material topology mismatch'):
            self.validate_modified(glb)

    def test_truncated_accessor_bounds_rejected(self):
        glb = self.source()
        index = glb.data['meshes'][0]['primitives'][0]['attributes']['POSITION']
        glb.data['accessors'][index]['count'] = 100000000
        with self.assertRaisesRegex(ValueError, 'buffer bounds'):
            self.validate_modified(glb)

    def test_provenance_corruption_rejected_before_unreal_import(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)/'ArtSource/Delivery_V80'
            shutil.copytree(FOLDER, folder)
            with (folder/'SM_AskirSupplyCrate.obj').open('a') as handle:
                handle.write('# altered source\n')
            with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
                IMPORTER.source_preflight(temporary)

    def test_forged_origin_bounds_cannot_pass_geometry_contract(self):
        metadata = copy.deepcopy(self.metadata['props'][VALIDATOR.NAMES[0]])
        metadata['bounds_metres_z_up'][0][2] = -1
        with self.assertRaisesRegex(ValueError, 'bounds mismatch'):
            VALIDATOR.validate_prop(FOLDER, VALIDATOR.NAMES[0], metadata)

    def test_import_path_guard_prevents_non_generated_package(self):
        with self.assertRaisesRegex(ValueError, 'unmanaged prop'):
            IMPORTER._package_file(ROOT, '/Game/AnythingElse')


if __name__ == '__main__':
    unittest.main()
