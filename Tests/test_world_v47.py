import json,sys,tempfile,shutil,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'Tools'))
from audit_world_v47 import audit
class WorldAuditTests(unittest.TestCase):
 def test_original(self):
  result=audit(ROOT)
  self.assertTrue(result['ok'],result['issues'])
  self.assertEqual((result['maps'],result['sites'],result['encounters'],result['anchors']),(30,150,90,240))
 def fixture(self,tmp):
  dest=Path(tmp)/'Production'/'Levels';dest.mkdir(parents=True)
  for name in ('level_designs_v42.json','world_manifest_v36.json','world_gameplay_anchors_v46.json'):
   shutil.copy2(ROOT/'Production'/'Levels'/name,dest/name)
  return dest
 def test_missing_anchor(self):
  with tempfile.TemporaryDirectory() as tmp:
   dest=self.fixture(tmp);p=dest/'world_gameplay_anchors_v46.json';obj=json.loads(p.read_text());obj['anchors'].pop();p.write_text(json.dumps(obj))
   self.assertFalse(audit(tmp)['ok'])
 def test_broken_exit(self):
  with tempfile.TemporaryDirectory() as tmp:
   dest=self.fixture(tmp);p=dest/'level_designs_v42.json';obj=json.loads(p.read_text());obj['levels'][0]['exits']=['L99_Missing'];p.write_text(json.dumps(obj))
   self.assertFalse(audit(tmp)['ok'])
 def test_changed_position(self):
  with tempfile.TemporaryDirectory() as tmp:
   dest=self.fixture(tmp);p=dest/'world_gameplay_anchors_v46.json';obj=json.loads(p.read_text());obj['anchors'][0]['position_cm'][0]+=100;p.write_text(json.dumps(obj))
   self.assertFalse(audit(tmp)['ok'])
if __name__=='__main__':unittest.main()
