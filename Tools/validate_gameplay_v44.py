import json, pathlib, sys
root=pathlib.Path(__file__).resolve().parents[1]
levels=json.loads((root/'Production/Levels/level_designs_v42.json').read_text(encoding='utf8'))['levels']
assert len(levels)==30
names={v['map_name'] for v in levels}
assert len(names)==30
ids=set()
for level in levels:
 assert len(level['sites'])==5 and len(level['encounters'])==3
 assert level['story_mission'] and level['exits']
 for target in level['exits']:assert target in names,(level['id'],target)
 for obj in level['sites']+level['encounters']:
  assert obj['id'] not in ids,obj['id'];ids.add(obj['id'])
  assert len(obj['position'])==3
  assert all(isinstance(n,(int,float)) for n in obj['position'])
for level in levels:
 for target in level['exits']:
  other=next(x for x in levels if x['map_name']==target)
  assert level['map_name'] in other['exits'],(level['map_name'],target)
assert len(ids)==240
print('PASS: 30 maps, 150 unique sites, 90 unique encounters, reciprocal exits')
