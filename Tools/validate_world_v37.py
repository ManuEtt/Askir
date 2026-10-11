"""Offline validation of Askir world manifest, map links and cook configuration."""
import json,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
levels=json.loads((root/'Production/Levels/world_manifest_v36.json').read_text(encoding='utf8'))['levels']
by_name={v['map_name']:v for v in levels}
assert len(levels)>=30 and len(by_name)==len(levels)
for level in levels:
 assert level['chapter'] in range(1,6)
 for dest in level['exits']:
  assert dest in by_name, (level['map_name'],dest)
  assert level['map_name'] in by_name[dest]['exits'], (level['map_name'],dest)
visited=set(); stack=[levels[0]['map_name']]
while stack:
 name=stack.pop()
 if name in visited:continue
 visited.add(name);stack.extend(by_name[name]['exits'])
assert visited==set(by_name), 'World graph contains unreachable maps'
cook=(root/'Config/WorldMapsToCook_V37.ini.txt').read_text(encoding='utf8')
for name in by_name:assert '/'+name+'"' in cook
print('PASS: 30 regions, reciprocal connections, reachability, cook list')
