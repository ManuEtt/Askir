"""Validate actual original V80 OBJ/GLB geometry, materials and provenance offline.

This checks source buffers, not Unreal packages, collision or runtime appearance.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import hashlib
import itertools
import json
import math
from pathlib import Path
import struct

import numpy as np

NAMES = ('SM_AskirDeliveryCart', 'SM_AskirSupplyCrate')
SUFFIXES = ('.blend', '.glb', '.obj', '.mtl', '_Preview.png')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def finite(values, label):
    require(all(math.isfinite(float(value)) for value in values), 'Non-finite '+label)


def read_obj(path):
    vertices, normals, triangles, material = [], [], [], None
    materials = {}
    current = None
    for line in path.with_suffix('.mtl').read_text(encoding='utf-8').splitlines():
        fields = line.split()
        if not fields or fields[0].startswith('#'):
            continue
        if fields[0] == 'newmtl':
            require(len(fields) == 2 and fields[1] not in materials, 'Invalid/duplicate MTL material')
            current = fields[1]
            materials[current] = {}
        elif fields[0] in ('Kd', 'Pr', 'Pm'):
            require(current is not None, 'MTL values precede material')
            values = list(map(float, fields[1:]))
            finite(values, 'MTL material')
            require(len(values) == (3 if fields[0] == 'Kd' else 1) and all(0 <= x <= 1 for x in values), 'Invalid MTL material values')
            materials[current][fields[0]] = values
    require(len(materials) >= 4 and all(set(value) == {'Kd', 'Pr', 'Pm'} for value in materials.values()), 'Incomplete original materials')
    for line in path.read_text(encoding='utf-8').splitlines():
        fields = line.split()
        if not fields or fields[0].startswith('#'):
            continue
        kind = fields[0]
        if kind == 'mtllib':
            require(fields[1:] == [path.with_suffix('.mtl').name], 'Unsafe or unexpected OBJ material path')
        elif kind in ('v', 'vn'):
            values = list(map(float, fields[1:]))
            require(len(values) == 3, 'OBJ coordinate must contain three values')
            finite(values, 'OBJ coordinates/normals')
            if kind == 'vn':
                require(abs(sum(x*x for x in values)-1) < .0001, 'Non-unit OBJ normal')
                normals.append(values)
            else:
                vertices.append(values)
        elif kind == 'usemtl':
            require(len(fields) == 2 and fields[1] in materials, 'Unknown OBJ material')
            material = fields[1]
        elif kind == 'f':
            require(len(fields) == 4 and material is not None, 'OBJ must have material-assigned triangles')
            indices = []
            for value in fields[1:]:
                pieces = value.split('//')
                require(len(pieces) == 2, 'Unexpected OBJ face format')
                index, normal = map(int, pieces)
                require(0 < index <= len(vertices) and 0 < normal <= len(normals), 'OBJ index out of range')
                indices.append(index-1)
            require(len(set(indices)) == 3, 'Repeated triangle vertex')
            triangles.append((tuple(indices), material))
    require(len(vertices) > 100 and len(triangles) > 100, 'Missing actual prop geometry')
    points = np.asarray(vertices, dtype=np.float64)
    for indices, _ in triangles:
        a, b, c = points[list(indices)]
        require(np.linalg.norm(np.cross(b-a, c-a)) > 1e-10, 'Degenerate OBJ triangle')
    return points, triangles, materials


class GLB:
    def __init__(self, path):
        raw = Path(path).read_bytes()
        require(len(raw) > 128, 'Missing GLB mesh')
        magic, version, size = struct.unpack_from('<4sII', raw)
        require(magic == b'glTF' and version == 2 and size == len(raw), 'Invalid GLB header/length')
        chunks, offset = {}, 12
        while offset < size:
            require(offset+8 <= size, 'Truncated GLB chunk')
            length, kind = struct.unpack_from('<I4s', raw, offset)
            offset += 8
            require(length % 4 == 0 and offset+length <= size and kind not in chunks, 'Invalid GLB chunks')
            chunks[kind] = raw[offset:offset+length]
            offset += length
        require(b'JSON' in chunks and b'BIN\x00' in chunks, 'Missing GLB JSON/binary')
        self.data = json.loads(chunks[b'JSON'])
        self.binary = chunks[b'BIN\x00']
        require(self.data.get('asset', {}).get('version') == '2.0', 'Invalid glTF version')
        require(len(self.data.get('buffers', [])) == 1 and 'uri' not in self.data['buffers'][0], 'External GLB buffers forbidden')
        require(self.data['buffers'][0]['byteLength'] <= len(self.binary), 'Truncated GLB binary')
        require(not any(self.data.get(key) for key in ('images', 'textures', 'skins', 'animations')), 'Unexpected external textures or character content')

    def accessor(self, index):
        require(isinstance(index, int) and 0 <= index < len(self.data.get('accessors', [])), 'Invalid accessor index')
        access = self.data['accessors'][index]
        require('sparse' not in access and not access.get('normalized'), 'Unexpected sparse/normalized mesh accessor')
        components = {'SCALAR': 1, 'VEC3': 3}.get(access.get('type'))
        dtype = {5121: '<u1', 5123: '<u2', 5125: '<u4', 5126: '<f4'}.get(access.get('componentType'))
        require(components is not None and dtype is not None, 'Unsupported accessor component/type')
        view_index = access.get('bufferView')
        require(isinstance(view_index, int) and 0 <= view_index < len(self.data.get('bufferViews', [])), 'Invalid buffer view')
        view = self.data['bufferViews'][view_index]
        require(view.get('buffer') == 0, 'Invalid GLB buffer reference')
        item_size = np.dtype(dtype).itemsize
        stride = view.get('byteStride', components*item_size)
        count = access.get('count', 0)
        offset = access.get('byteOffset', 0)
        view_offset, view_length = view.get('byteOffset', 0), view.get('byteLength', 0)
        require(all(isinstance(value, int) and value >= 0 for value in (count, offset, view_offset, view_length, stride)), 'Invalid accessor bounds')
        require(count > 0 and stride >= components*item_size, 'Empty/overlapping accessor')
        needed = offset+(count-1)*stride+components*item_size
        require(needed <= view_length and view_offset+view_length <= len(self.binary), 'Accessor exceeds buffer bounds')
        return np.ndarray((count, components), dtype=dtype, buffer=self.binary,
                          offset=view_offset+offset, strides=(stride, item_size)).copy()


def validate_prop(folder, name, metadata):
    obj_points, obj_faces, materials = read_obj(folder/(name+'.obj'))
    glb = GLB(folder/(name+'.glb'))
    data = glb.data
    require(len(data.get('nodes', [])) == 1 and data['nodes'][0].get('mesh') == 0, 'Expected one static mesh node')
    require(not any(key in data['nodes'][0] for key in ('matrix', 'translation', 'rotation', 'scale', 'children')), 'Unexpected mesh transform')
    require(len(data.get('meshes', [])) == 1, 'Expected one original prop mesh')
    # Canonical OBJ vertex IDs collapse coincident split vertices; glTF legitimately
    # duplicates vertices at hard normals/material seams. Compare triangle multisets.
    tolerance = 2e-6
    cell_size = 1e-5
    cells = defaultdict(list)
    canonical = []
    for point in obj_points:
        cell = tuple(np.floor(point/cell_size).astype(np.int64))
        matches = [(index, p) for index, p in cells[cell] if np.max(np.abs(point-p)) < tolerance]
        index = matches[0][0] if matches else len(canonical)
        if not matches:
            canonical.append(point)
            cells[cell].append((index, point))
    def match(point):
        cell = tuple(np.floor(point/cell_size).astype(np.int64))
        for delta in itertools.product((-1, 0, 1), repeat=3):
            for index, candidate in cells[tuple(a+b for a, b in zip(cell, delta))]:
                if np.max(np.abs(point-candidate)) < tolerance:
                    return index
        raise ValueError('GLB/OBJ vertex geometry mismatch')
    obj_counter = Counter((tuple(sorted(match(obj_points[i]) for i in indices)), mat) for indices, mat in obj_faces)
    glb_counter = Counter()
    all_points = []
    for primitive in data['meshes'][0].get('primitives', []):
        require(primitive.get('mode', 4) == 4, 'GLB primitive must contain triangles')
        position_accessor = primitive.get('attributes', {}).get('POSITION')
        require(data['accessors'][position_accessor]['componentType'] == 5126, 'GLB position must use float32')
        positions = glb.accessor(position_accessor).astype(np.float64)
        require(positions.shape[1] == 3 and np.isfinite(positions).all(), 'Non-finite GLB positions')
        normals = glb.accessor(primitive.get('attributes', {}).get('NORMAL'))
        require(normals.shape == positions.shape and np.isfinite(normals).all()
                and np.allclose(np.linalg.norm(normals, axis=1), 1, atol=.0001), 'Invalid/non-finite GLB normals')
        # Blender +Z up -> standard glTF +Y up: (x,y,z) -> (x,z,-y).
        positions = positions[:, (0, 2, 1)] * np.array((1, -1, 1))
        indices = glb.accessor(primitive['indices']).reshape(-1)
        require(indices.dtype.kind == 'u' and len(indices) % 3 == 0 and int(indices.max()) < len(positions), 'Invalid GLB triangle indices')
        mat_index = primitive.get('material')
        require(isinstance(mat_index, int) and 0 <= mat_index < len(data.get('materials', [])), 'Missing GLB material')
        mat = data['materials'][mat_index]
        mat_name = mat.get('name')
        require(mat_name in materials, 'GLB/OBJ material mismatch')
        pbr = mat.get('pbrMetallicRoughness', {})
        values = pbr.get('baseColorFactor', [])
        require(len(values) == 4 and all(math.isfinite(x) and 0 <= x <= 1 for x in values), 'Invalid GLB material color')
        require(np.allclose(values[:3], materials[mat_name]['Kd'], atol=1e-5), 'GLB/MTL color mismatch')
        for gltf_key, mtl_key in (('roughnessFactor', 'Pr'), ('metallicFactor', 'Pm')):
            value = pbr.get(gltf_key, 1)
            require(math.isfinite(value) and abs(value-materials[mat_name][mtl_key][0]) < 1e-5, 'GLB/MTL material property mismatch')
        mapped = [match(point) for point in positions]
        for a, b, c in indices.reshape(-1, 3):
            triangle = positions[[a, b, c]]
            require(np.linalg.norm(np.cross(triangle[1]-triangle[0], triangle[2]-triangle[0])) > 1e-10, 'Degenerate GLB triangle')
            glb_counter[(tuple(sorted((mapped[a], mapped[b], mapped[c]))), mat_name)] += 1
        all_points.extend(positions)
    require(glb_counter == obj_counter, 'GLB/OBJ triangle or material topology mismatch')
    bounds = np.stack((obj_points.min(axis=0), obj_points.max(axis=0)))
    require(np.allclose(bounds, metadata['bounds_metres_z_up'], atol=2e-6), 'Source/provenance bounds mismatch')
    require(len(obj_points) == metadata['vertices'] and len(obj_faces) == metadata['triangles'] and len(materials) == metadata['materials'], 'Source/provenance geometry counts mismatch')
    require(np.allclose((bounds[1]-bounds[0])*100, metadata['dimensions_centimetres'], atol=.001), 'Incorrect metre/centimetre contract')
    require(-.001 <= bounds[0, 2] <= .03 and bounds[1, 2] > .5, 'Unexpected ground origin or prop height')
    require((folder/(name+'.blend')).read_bytes().startswith(b'BLENDER'), 'Missing Blender source file')
    png = (folder/(name+'_Preview.png')).read_bytes()
    require(png[:8] == b'\x89PNG\r\n\x1a\n' and struct.unpack_from('>II', png, 16) == (960, 720), 'Missing actual preview PNG')
    return {'vertices': len(obj_points), 'triangles': len(obj_faces), 'materials': len(materials),
            'bounds_metres_z_up': bounds.tolist(), 'glb_obj_triangle_geometry': 'Bestanden'}


def validate(project_root):
    folder = Path(project_root)/'ArtSource/Delivery_V80'
    metadata = json.loads((folder/'PROVENANCE.json').read_text(encoding='utf-8'))
    require(metadata.get('license') == 'CC0-1.0' and metadata.get('unreal_obj_uniform_scale') == 100, 'Invalid source license/scale provenance')
    expected = {name+suffix for name in NAMES for suffix in SUFFIXES} | {'LICENSE_CC0.txt'}
    require(set(metadata.get('files', {})) == expected, 'Incomplete source provenance inventory')
    for name, item in metadata['files'].items():
        raw = (folder/name).read_bytes()
        require(len(raw) == item['bytes'] and hashlib.sha256(raw).hexdigest() == item['sha256'], 'Source checksum mismatch: '+name)
    require(set(metadata.get('props', {})) == set(NAMES), 'Unexpected prop inventory')
    props = {name: validate_prop(folder, name, metadata['props'][name]) for name in NAMES}
    return {'status': 'Bestanden', 'coverage': 'Actual OBJ/GLB vertices, nondegenerate triangles, material topology/colors, source bounds and all SHA256 hashes',
            'unreal_import': 'Nicht ausgeführt', 'runtime_collision': 'Nicht ausgeführt', 'props': props}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--report', type=Path)
    args = parser.parse_args(argv)
    try:
        result = validate(args.project_root)
        code = 0
    except Exception as exc:
        result, code = {'status': 'Fehlgeschlagen', 'error': str(exc), 'unreal_import': 'Nicht ausgeführt'}, 1
    report = args.report or args.project_root/'Reports/V80_DELIVERY_PROP_SOURCE_VALIDATION.json'
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps(result, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print('ASKIR V80 delivery source validation:', result['status'], report)
    return code


if __name__ == '__main__':
    raise SystemExit(main())
