"""Offline validation of Dornfeld's first quest contract (no Unreal dependency)."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MISSION = ROOT / 'Production/Quests/dornfeld_first_playable_mission.json'
EXPECTED = [('TALK_ELDER','dialogue'),('COLLECT_SHARDS','collect'),('DEFEAT_GUARDIANS','encounter'),('OPEN_CHEST','loot'),('RETURN_TO_ELDER','dialogue')]


def validate_mission(data):
    errors = []
    stages = data.get('stages', [])
    actual = [(s.get('id'), s.get('type')) for s in stages]
    if actual != EXPECTED:
        errors.append('Stage order or type differs from the required playable sequence')
    if len({s.get('id') for s in stages}) != len(stages):
        errors.append('Duplicate stage IDs')
    by_id = {s.get('id'): s for s in stages}
    for name, expected in [('COLLECT_SHARDS',3),('DEFEAT_GUARDIANS',2)]:
        if by_id.get(name, {}).get('count') != expected:
            errors.append(f'{name} requires count {expected}')
    for name in ('TALK_ELDER','RETURN_TO_ELDER'):
        if by_id.get(name, {}).get('npc') != 'Dorfälteste Mara':
            errors.append(f'{name} must use Mara')
    if data.get('reward',{}).get('gold') != 90 or data.get('reward',{}).get('xp') != 180:
        errors.append('Quest rewards must be exactly 90 gold and 180 XP')
    if not data.get('id'):
        errors.append('Missing stable quest ID')
    return errors


def main():
    data = json.loads(MISSION.read_text(encoding='utf-8'))
    errors = validate_mission(data)
    report = {'version':73,'mission':data.get('id'),'contract_valid':not errors,'errors':errors,
              'unreal_editor_execution':'NOT_RUN','end_to_end_playable':'NOT_VERIFIED'}
    path = ROOT / 'Reports/V73_DORNFELD_PREFLIGHT.json'
    path.parent.mkdir(exist_ok=True)
    path.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,ensure_ascii=False))
    return int(bool(errors))

if __name__ == '__main__':
    raise SystemExit(main())
