import json,sys,unittest,tempfile,pathlib
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/'Tools'))
from validate_world_v39 import validate
ROOT=pathlib.Path(__file__).resolve().parents[1]
class WorldTests(unittest.TestCase):
 def test_original_manifest(self):self.assertEqual([],validate(ROOT/'Production/Levels/world_manifest_v36.json'))
 def test_missing_exit(self):
  data=json.loads((ROOT/'Production/Levels/world_manifest_v36.json').read_text())
  data['levels'][0]['exits'].append('nonexistent')
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t)/'bad.json';p.write_text(json.dumps(data));self.assertTrue(validate(p))
 def test_duplicate(self):
  data=json.loads((ROOT/'Production/Levels/world_manifest_v36.json').read_text())
  data['levels'][1]['map_name']=data['levels'][0]['map_name']
  with tempfile.TemporaryDirectory() as t:
   p=pathlib.Path(t)/'bad.json';p.write_text(json.dumps(data));self.assertTrue(validate(p))
if __name__=='__main__':unittest.main()
