import importlib.util, pathlib, tempfile, shutil, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('preflight',ROOT/'Tools/preflight_world_v45.py')
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
class WorldPipelineTests(unittest.TestCase):
 def test_current_project(self):self.assertEqual(mod.validate(ROOT),[])
 def test_detect_bad_cook_path(self):
  with tempfile.TemporaryDirectory() as temp:
   root=pathlib.Path(temp)
   for relative in ['Production/Levels/world_manifest_v36.json','Production/Levels/level_designs_v42.json','Tools/generate_30_world_blockouts_v36.py','Tools/setup_world_portals_v43.py','Tools/integrate_30_level_gameplay_v44.py','Source/Askir/Travel/AskirWorldPortal.cpp','Config/WorldMapsToCook_V45.ini.txt']:
    (root/relative).parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/relative,root/relative)
   p=root/'Config/WorldMapsToCook_V45.ini.txt';p.write_text(p.read_text().replace('/Game/Askir/WorldV42/','/Game/Bad/'))
   self.assertTrue(any('Cook list' in e for e in mod.validate(root)))
 def test_detect_bad_generator_path(self):
  with tempfile.TemporaryDirectory() as temp:
   root=pathlib.Path(temp)
   for relative in ['Production/Levels/world_manifest_v36.json','Production/Levels/level_designs_v42.json','Tools/generate_30_world_blockouts_v36.py','Tools/setup_world_portals_v43.py','Tools/integrate_30_level_gameplay_v44.py','Source/Askir/Travel/AskirWorldPortal.cpp','Config/WorldMapsToCook_V45.ini.txt']:
    (root/relative).parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/relative,root/relative)
   p=root/'Tools/generate_30_world_blockouts_v36.py';p.write_text(p.read_text().replace("OUTPUT_ROOT = '/Game/Askir/WorldV42'","OUTPUT_ROOT = '/Game/Bad'"))
   self.assertTrue(any('generate_30_world_blockouts' in e for e in mod.validate(root)))
if __name__=='__main__':unittest.main()
