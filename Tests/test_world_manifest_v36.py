"""Run with Python 3: python Tests/test_world_manifest_v36.py"""
import json, pathlib, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
DATA=json.loads((ROOT/'Production/Levels/world_manifest_v36.json').read_text(encoding='utf-8'))
class WorldManifestTests(unittest.TestCase):
 def test_thirty_unique_levels(self):
  levels=DATA['levels'];self.assertEqual(len(levels),30)
  self.assertEqual(len({x['map_name'] for x in levels}),30)
 def test_story_chapters(self):
  self.assertEqual({x['chapter'] for x in DATA['levels']},{1,2,3,4,5})
  self.assertEqual({x['continent'] for x in DATA['levels']},{'Ardem','Thalor','Velis'})
 def test_links_are_valid_and_symmetric(self):
  d={x['map_name']:x for x in DATA['levels']}
  for item in DATA['levels']:
   for neighbor in item['exits']:
    self.assertIn(neighbor,d)
    self.assertIn(item['map_name'],d[neighbor]['exits'])
 def test_all_maps_reachable(self):
  d={x['map_name']:x for x in DATA['levels']};seen=set();todo=[DATA['levels'][0]['map_name']]
  while todo:
   key=todo.pop()
   if key in seen:continue
   seen.add(key);todo.extend(d[key]['exits'])
  self.assertEqual(len(seen),30)
 def test_story_and_pois(self):
  for item in DATA['levels']:
   self.assertRegex(item['story_mission'],r'^M0[1-5]-0[1-4]$')
   self.assertGreaterEqual(len(item['points_of_interest']),2)
   self.assertEqual(item['implementation_status'],'editor_generation_pending')
if __name__=='__main__':unittest.main()
