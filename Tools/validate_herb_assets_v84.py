"""Validate real V84 herb source geometry; this never creates Unreal assets."""
from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def validate(root=ROOT):
    root = Path(root)
    importer = module('_v84_herb_source_preflight', root/'Tools/import_herb_assets_v84.py')
    source = importer.source_preflight(root)
    # Reuse the existing actual triangle/material topology validator, rather
    # than accepting headers or authoring metadata as geometry proof.
    topology = module('_v84_original_prop_geometry', root/'Tools/validate_delivery_props_v80.py')
    folder = root/'ArtSource/Environment_V84'
    record = source['props'][importer.NAMES[0]]
    geometry = topology.validate_prop(folder, importer.SOURCE_NAME, record)
    _, _, materials = topology.read_obj(folder/(importer.SOURCE_NAME+'.obj'))
    for name, values in materials.items():
        original = source['materials'][name]
        for actual, expected in zip(values['Kd']+[values['Pr'][0], values['Pm'][0]],
                                    original['base_color'][:3]+[original['roughness'], original['metallic']]):
            if abs(actual-expected) > 1e-5:
                raise ValueError('Herb MTL/PBR metadata values differ')
    return {'status': 'Bestanden', 'version': 'V84', 'geometry': geometry, 'files': source['files'],
            'coverage': 'Actual GLB/OBJ vertices, indices, nondegenerate triangle topology, normals, material colors, floor origin and all source SHA256 hashes; FBX/Blender headers',
            'unreal_import': 'Nicht ausgeführt', 'package_reload': 'Nicht ausgeführt', 'runtime_collision': 'Nicht ausgeführt'}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-root', type=Path, default=ROOT)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args(argv)
    try:
        result, code = validate(args.project_root), 0
    except Exception as error:
        result, code = {'status': 'Fehlgeschlagen', 'error': str(error), 'unreal_import': 'Nicht ausgeführt'}, 1
    report = args.report or args.project_root/'Reports/V84/herb_asset_source_validation.json'
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(result, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print('ASKIR V84 healing herb source validation:', result['status'], report)
    return code


if __name__ == '__main__':
    raise SystemExit(main())
