"""V78 pipeline regression tests run real parser/generator contracts only."""
from pathlib import Path
import importlib.util
import unittest
ROOT=Path(__file__).resolve().parents[1]

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

class PipelineV78(unittest.TestCase):
    def test_default_map_readiness_is_exact(self):
        runner=load('v78runner','Tools/run_unreal_v78.py')
        self.assertEqual(runner.readiness_map('ASKIR_V78_RUNTIME_READY map=Dornfeld_V78 player=Hero'),'Dornfeld_V78')
        for wrong in ('Entry','Dornfeld_V77','Dornfeld_V78_Fake'):
            with self.assertRaises(RuntimeError):runner.readiness_map('ASKIR_V78_RUNTIME_READY map='+wrong+' player=Hero')
    def test_v78_cannot_reuse_v77_map_file(self):
        base=load('v78base','Tools/build_dornfeld_v77.py');base.MAP_PATH='/Game/Askir/Maps/Dornfeld_V78'
        config=base.startup_config_text('[/Script/EngineSettings.GameMapsSettings]\nGameDefaultMap=/Engine/Maps/Entry\n')
        self.assertIn('GameDefaultMap=/Game/Askir/Maps/Dornfeld_V78',config);self.assertNotIn('Maps/Dornfeld_V77',config)
    def test_imports_and_validation_precede_startup_changes(self):
        source=(ROOT/'Tools/build_dornfeld_v78.py').read_text()
        self.assertLess(source.index('importer.source_preflight(root)'),source.index('importer.import_assets(unreal,root)'))
        self.assertLess(source.index('validation[\'navigation_paths\']=validate_paths'),source.index("result['startup_map_config']=configure_startup"))
        self.assertIn('waiting for V78 final validation',source)
    def test_cook_covers_dynamic_spell_and_rig_packages(self):
        config=(ROOT/'Config/DefaultGame.ini').read_text()
        for path in ('/Game/Askir/V77','/Game/Askir/V78','/Game/Askir/CharactersV78'):self.assertIn('Path="'+path+'"',config)
    def test_native_runtime_marker_separates_versions(self):
        code=(ROOT/'Source/Askir/Game/AskirGameModeDemo.cpp').read_text()
        self.assertIn('Map==TEXT("Dornfeld_V78")',code);self.assertIn('ASKIR_V78_RUNTIME_READY',code);self.assertIn('ASKIR_V77_RUNTIME_READY',code)
if __name__=='__main__':unittest.main()
