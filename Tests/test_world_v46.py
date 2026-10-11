import sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'Tools'))
from validate_world_gameplay_v46 import validate
class GameplayAnchors(unittest.TestCase):
 def test_manifest(self):self.assertEqual(validate(ROOT),[])
 def test_reject_duplicate(self):
  import tempfile,shutil,json
  with tempfile.TemporaryDirectory() as tmp:
   t=Path(tmp);(t/'Production/Levels').mkdir(parents=True)
   for name in ('level_designs_v42.json','world_gameplay_anchors_v46.json'):
    shutil.copy(ROOT/'Production/Levels'/name,t/'Production/Levels'/name)
   p=t/'Production/Levels/world_gameplay_anchors_v46.json';d=json.loads(p.read_text());d['anchors'][1]['site_id']=d['anchors'][0]['site_id'];p.write_text(json.dumps(d))
   self.assertTrue(any('Duplicate' in e for e in validate(t)))
 def test_reject_missing_anchor(self):
  import tempfile,shutil,json
  with tempfile.TemporaryDirectory() as tmp:
   t=Path(tmp);(t/'Production/Levels').mkdir(parents=True)
   for name in ('level_designs_v42.json','world_gameplay_anchors_v46.json'):
    shutil.copy(ROOT/'Production/Levels'/name,t/'Production/Levels'/name)
   p=t/'Production/Levels/world_gameplay_anchors_v46.json';d=json.loads(p.read_text());d['anchors'].pop();p.write_text(json.dumps(d))
   self.assertTrue(validate(t))
if __name__=='__main__':unittest.main()
