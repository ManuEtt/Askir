from pathlib import Path
import json,csv
root=Path(__file__).resolve().parents[1]
def load(p):return json.loads((root/p).read_text(encoding='utf-8'))
q=load('Production/Quests/sidequests.json'); items=load('Production/Items/items.json'); spells=load('Production/Magic/spells.json'); levels=load('Production/Levels/level_layouts.json')
assert len(q)>=20 and len(levels)==5 and len(spells)>=20 and len(items)>=50
assert len({x['id'] for x in q})==len(q)
ids={x['id'] for x in items}
assert all(x['rewards']['item'] in ids for x in q)
assert all(x['objectives'] and x['rewards']['xp']>0 for x in q)
for name in ('quests','spells','items'):
 with (root/f'Production/DataTables/DT_{name}.csv').open(encoding='utf-8-sig',newline='') as f:assert len(list(csv.DictReader(f)))>0
print('V28: 10 content and reference checks OK')
