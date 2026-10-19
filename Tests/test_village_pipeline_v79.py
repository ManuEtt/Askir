"""Offline V79 data/pipeline boundary checks; no map or gameplay success claims."""
from __future__ import annotations
import copy
import importlib.util
import json
import math
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest

ROOT = Path(__file__).resolve().parents[1]


def load(name, file):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'Tools' / file)
    result = importlib.util.module_from_spec(spec);spec.loader.exec_module(result)
    return result


CATALOG = load('_v79_village_catalog_tests', 'generate_village_catalog_v79.py')
BUILDER = load('_v79_village_builder_tests', 'build_dornfeld_v79.py')
RUNNER = load('_v79_village_runner_tests', 'run_unreal_v79.py')


class VillageDataBoundaries(unittest.TestCase):
    def setUp(self):
        self.data = copy.deepcopy(CATALOG.load_contract())

    def rejected(self, change):
        change(self.data)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'catalog.json';path.write_text(json.dumps(self.data), encoding='utf-8')
            with self.assertRaises((ValueError, KeyError)): CATALOG.load_contract(path)

    def test_duplicate_item_ids_rejected(self):
        self.rejected(lambda data: data['items'].append(copy.deepcopy(data['items'][0])))

    def test_duplicate_persistent_pickup_ids_rejected(self):
        self.rejected(lambda data: data['resource_pickups'].append(copy.deepcopy(data['resource_pickups'][0])))

    def test_unresolved_ingredient_rejected(self):
        self.rejected(lambda data: data['recipes'][0]['ingredients'][0].update(item_id='missing_item'))

    def test_quest_items_cannot_be_traded_or_crafted(self):
        self.rejected(lambda data: data['items'][0].update(type='Quest'))

    def test_negative_prices_and_stock_rejected(self):
        for field in ('value', 'stock', 'max_stock'):
            with self.subTest(field=field):
                self.setUp();self.rejected(lambda data: data['items'][0].update({field: -1}))

    def test_nonfinite_properties_and_coordinates_rejected(self):
        for value in (math.nan, math.inf, -math.inf):
            with self.subTest(value=value):
                self.setUp();self.rejected(lambda data: data['items'][0].update(weight=value))
                self.setUp();self.rejected(lambda data: data.update(merchant_position=[value, 0, 90]))

    def test_consumable_without_gameplay_effect_rejected(self):
        self.rejected(lambda data: next(item for item in data['items'] if item['type']=='Consumable').update(heal=0))

    def test_recipe_knowledge_is_a_boolean(self):
        self.rejected(lambda data: data['recipes'][0].update(known_by_default='true'))

    def test_self_producing_recipe_rejected(self):
        self.rejected(lambda data: data['recipes'][0].update(result=copy.deepcopy(data['recipes'][0]['ingredients'][0])))

    def test_duplicate_recipe_ingredients_rejected(self):
        self.rejected(lambda data: data['recipes'][0]['ingredients'].append(copy.deepcopy(data['recipes'][0]['ingredients'][0])))

    def test_zero_batch_output_rejected(self):
        self.rejected(lambda data: data['recipes'][0]['result'].update(quantity=0))

    def test_native_and_editor_share_exact_catalog(self):
        expected=CATALOG.generated_text(self.data)
        self.assertEqual(CATALOG.GENERATED.read_text(encoding='utf-8'), expected)
        self.assertEqual(BUILDER.source_preflight(ROOT), self.data)


class VillagePipelineBoundaries(unittest.TestCase):
    def test_approach_point_uses_actual_distance(self):
        def vector(x,y,z): return SimpleNamespace(x=x,y=y,z=z)
        unreal=SimpleNamespace(Vector=vector)
        point=BUILDER.approach_point(unreal, vector(1000, 0, 100), vector(0, 0, 0), 190)
        self.assertAlmostEqual(math.sqrt(point.x**2 + point.y**2 + point.z**2), 190)
        self.assertAlmostEqual(point.x/point.z, 10)

    def test_invalid_navigation_endpoints_fail_closed(self):
        unreal=SimpleNamespace(Vector=lambda x,y,z: (x,y,z))
        zero=SimpleNamespace(x=0,y=0,z=0)
        for bad in (zero, SimpleNamespace(x=math.nan,y=0,z=0)):
            with self.assertRaises(RuntimeError): BUILDER.approach_point(unreal, bad, zero, 190)

    def test_startup_requires_both_economy_and_navigation(self):
        calls=[]
        village=SimpleNamespace(configure_startup_maps=lambda *args: calls.append(args))
        for validation in ({}, {'economy': {'status':'Bestanden'}}, {'navigation_paths': {'status':'Bestanden'}}, {'economy': {'status':'Fehlgeschlagen'}, 'navigation_paths': {'status':'Bestanden'}}):
            with self.assertRaises(RuntimeError): BUILDER.configure_startup(ROOT, village, validation)
        self.assertEqual(calls, [])

    def test_v79_runtime_marker_cannot_accept_other_maps(self):
        self.assertEqual(RUNNER.readiness_map('ASKIR_V79_RUNTIME_READY map=Dornfeld_V79 player=Hero'), 'Dornfeld_V79')
        for map_name in ('Entry', 'Dornfeld_V78', 'Dornfeld_V79_Fake'):
            with self.assertRaises(RuntimeError): RUNNER.readiness_map('ASKIR_V79_RUNTIME_READY map='+map_name)
        self.assertIsNone(RUNNER.readiness_map('ASKIR_V78_RUNTIME_READY map=Dornfeld_V78'))

    def test_preflight_is_honest_about_editor_and_gameplay(self):
        with tempfile.TemporaryDirectory() as directory:
            report=Path(directory)/'preflight.json'
            result=subprocess.run([sys.executable, str(ROOT/'Tools/build_dornfeld_v79.py'), '--preflight', '--report', str(report)], cwd=ROOT, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            data=json.loads(report.read_text())
            self.assertEqual(data['status'], 'Bestanden')
            self.assertEqual(data['editor_status'], 'Nicht ausgeführt')
            self.assertEqual(data['gameplay_status'], 'Nicht ausgeführt')
            self.assertNotIn('map_file', data)

    @unittest.skipIf(importlib.util.find_spec('unreal') is not None, 'Standalone test needs a host without Unreal Python')
    def test_missing_editor_is_blocked_and_creates_no_map(self):
        before=set((ROOT/'Content').rglob('*.umap'))
        with tempfile.TemporaryDirectory() as directory:
            report=Path(directory)/'editor.json'
            result=subprocess.run([sys.executable, str(ROOT/'Tools/build_dornfeld_v79.py'), '--report', str(report)], cwd=ROOT, capture_output=True, text=True)
            self.assertEqual(result.returncode, 2, result.stderr)
            data=json.loads(report.read_text())
            self.assertEqual(data['status'], 'Blockiert')
            self.assertEqual(data['gameplay_status'], 'Nicht ausgeführt')
        self.assertEqual(before, set((ROOT/'Content').rglob('*.umap')))

if __name__=='__main__':unittest.main()
