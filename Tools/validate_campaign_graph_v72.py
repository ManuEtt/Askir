"""Offline validation of ASKIR's canonical level transition graph.

Run: python Tools/validate_campaign_graph_v72.py
No Unreal installation required. Exit code 1 indicates campaign graph errors.
"""
import json
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'Production/Levels/world_manifest_v36.json'
REPORT = ROOT / 'Production/Levels/campaign_graph_audit_v72.json'


def audit(data):
    levels = data.get('levels', [])
    errors, warnings = [], []
    names = [item.get('map_name') for item in levels]
    if len(names) != len(set(names)):
        errors.append('Duplicate map_name values')
    by_name = {item['map_name']: item for item in levels if item.get('map_name')}
    if len(levels) < 30:
        errors.append(f'Only {len(levels)} maps specified; minimum is 30')
    for name, spec in by_name.items():
        if not spec.get('continent') or not spec.get('chapter'):
            errors.append(f'{name}: missing continent/chapter')
        exits = spec.get('exits', [])
        if len(exits) != len(set(exits)):
            errors.append(f'{name}: duplicate exits')
        for dest in exits:
            if dest == name:
                errors.append(f'{name}: self-transition')
            elif dest not in by_name:
                errors.append(f'{name}: unknown destination {dest}')
            elif name not in by_name[dest].get('exits', []):
                warnings.append(f'{name} -> {dest}: one-way transition; verify intentional')
    start = 'L01_Dornfeld'
    visited = set()
    if start not in by_name:
        errors.append('Missing starting map L01_Dornfeld')
    else:
        pending = deque([start])
        while pending:
            current = pending.popleft()
            if current in visited:
                continue
            visited.add(current)
            pending.extend(dest for dest in by_name[current].get('exits', []) if dest in by_name and dest not in visited)
    unreachable = sorted(set(by_name) - visited)
    if unreachable:
        errors.append('Unreachable from Dornfeld: ' + ', '.join(unreachable))
    return {'schema_version': 1, 'scope': 'manifest_graph_only', 'level_count': len(levels),
            'reachable_count': len(visited), 'errors': errors, 'warnings': warnings,
            'unreal_levels_verified': False, 'playability_verified': False,
            'status': 'passed' if not errors else 'failed'}


def main():
    result = audit(json.loads(MANIFEST.read_text(encoding='utf-8')))
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if not result['errors'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
