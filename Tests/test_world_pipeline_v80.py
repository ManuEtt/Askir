"""Behavioral boundaries for V80 source contracts and fail-closed engine evidence.

Temporary package-size fixtures exercise report rejection only. They are not
Unreal assets and are removed automatically; no map/gameplay validation is claimed.
"""
from __future__ import annotations
import copy
import importlib.util
import json
import math
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest

ROOT = Path(__file__).resolve().parents[1]


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name,ROOT/'Tools'/filename)
    result = importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result


BUILDER = load('_v80_world_builder_test','build_dornfeld_v80.py')
RUNNER = load('_v80_world_runner_test','run_unreal_v80.py')
DATA = json.loads((ROOT/BUILDER.CONTRACT_PATH).read_text())
ITEM = next(item for item in json.loads((ROOT/'Production/Items/items.json').read_text()) if item['id']=='item_002')
QUEST = next(quest for quest in json.loads((ROOT/'Production/Quests/sidequests.json').read_text()) if quest['id']=='SQ_001')


class WorldContractBoundaries(unittest.TestCase):
    def setUp(self): self.data = copy.deepcopy(DATA)

    def reject(self, change):
        change(self.data)
        with self.assertRaises((ValueError,KeyError)): BUILDER.validate_contract(self.data,ITEM,QUEST)

    def test_full_preflight_resolves_canonical_reward_and_existing_sources(self):
        self.assertEqual(BUILDER.source_preflight(ROOT),DATA)
        self.assertEqual(QUEST['rewards'],{'xp':100,'gold':16,'item':'item_002'})

    def test_single_map_cannot_satisfy_route_contract(self): self.reject(lambda data:data['maps'].pop())
    def test_legacy_map_cannot_substitute_for_v80(self): self.reject(lambda data:data['maps'][0].update(package='/Game/Askir/Maps/Dornfeld_V79'))
    def test_duplicate_portal_ids_rejected(self): self.reject(lambda data:data['maps'][1]['portal'].update(id=data['maps'][0]['portal']['id']))
    def test_nonreciprocal_destination_id_rejected(self): self.reject(lambda data:data['maps'][0]['portal'].update(destination_id='Missing'))
    def test_nonreciprocal_destination_package_rejected(self): self.reject(lambda data:data['maps'][0]['portal'].update(destination_package=data['maps'][0]['package']))
    def test_arrival_inside_trigger_rejected(self): self.reject(lambda data:data['maps'][1]['portal'].update(arrival_offset=[0,100,0]))
    def test_arrival_above_or_outside_ground_rejected(self):
        for offset in ([0,20000,0],[0,420,1000]):
            with self.subTest(offset=offset): self.data=copy.deepcopy(DATA);self.reject(lambda data:data['maps'][1]['portal'].update(arrival_offset=offset))
    def test_nonfinite_and_boolean_vectors_rejected(self):
        for value in (math.nan,math.inf,-math.inf,True):
            with self.subTest(value=value): self.data=copy.deepcopy(DATA);self.reject(lambda data:data['bandits'][0].update(position=[value,1900,96]))
    def test_negative_trigger_extent_rejected(self): self.reject(lambda data:data['maps'][1]['portal'].update(trigger_extent=[120,-220,220]))
    def test_duplicate_bandit_receipts_rejected(self): self.reject(lambda data:data['bandits'][1].update(id=data['bandits'][0]['id']))
    def test_cargo_cannot_use_bandit_receipt(self): self.reject(lambda data:data['cargo'].update(event_id='SQ_001_BANDIT_1'))
    def test_preserved_village_geometry_cannot_drift_from_parent_builder(self): self.reject(lambda data:data['maps'][0].update(ground_extent=[100,100,50]))
    def test_countryside_geometry_cannot_drop_authored_farms_outside_collision(self): self.reject(lambda data:data['maps'][1].update(ground_extent=[800,4500,50]))
    def test_canonical_reward_drift_rejected(self):
        for field,value in (('value',18),('stack_max',1),('weight',.7),('category','quest')):
            with self.subTest(field=field):
                item=copy.deepcopy(ITEM);item[field]=value
                with self.assertRaises(ValueError): BUILDER.validate_contract(self.data,item,QUEST)
    def test_canonical_quest_reward_drift_rejected(self):
        quest=copy.deepcopy(QUEST);quest['rewards']['xp']=200
        with self.assertRaises(ValueError): BUILDER.validate_contract(self.data,ITEM,quest)
    def test_rotated_mirrored_or_scaled_portals_cannot_invalidate_arrival_math(self):
        BUILDER.validate_portal_transform([0,0,0],[1,1,1])
        for rotation,scale in (([0,90,0],[1,1,1]),([0,0,0],[1,-1,1]),([0,0,0],[1,2,1])):
            with self.subTest(rotation=rotation,scale=scale),self.assertRaises(RuntimeError): BUILDER.validate_portal_transform(rotation,scale)
    def test_nonfinite_saved_portal_transforms_rejected(self):
        for rotation,scale in (([0,math.nan,0],[1,1,1]),([0,0,0],[1,math.inf,1])):
            with self.subTest(rotation=rotation,scale=scale),self.assertRaises(ValueError): BUILDER.validate_portal_transform(rotation,scale)


class WorldEvidenceBoundaries(unittest.TestCase):
    def setUp(self):
        self.directory=tempfile.TemporaryDirectory();self.root=Path(self.directory.name)
        for path in ('Production','Tools','ArtSource'):(self.root/path).symlink_to(ROOT/path,target_is_directory=True)
        catalog=self.root/'Source/Askir/Economy';catalog.parent.mkdir(parents=True);catalog.symlink_to(ROOT/'Source/Askir/Economy',target_is_directory=True)
        (self.root/'Config').mkdir();(self.root/'Config/DefaultEngine.ini').write_text('[/Script/EngineSettings.GameMapsSettings]\nGameDefaultMap='+BUILDER.MAP_PATH+'\nEditorStartupMap='+BUILDER.MAP_PATH+'\n')
        skeletons=['/Game/Askir/CharactersV78/'+role+'/SK_Askir'+role+'_Skeleton' for role in ('Hero','Guard','Raider')]
        packages=[DATA['reward_asset']]+skeletons+list(BUILDER.PROP_PACKAGES.values())
        for role in ('Hero','Guard','Raider'):
            packages+=['/Game/Askir/CharactersV78/'+role+'/SK_Askir'+role]
            packages+=['/Game/Askir/CharactersV78/'+role+'/A_'+clip for clip in ('Idle','Walk','Run','Sprint','Jump','Dodge','LightAttack','HeavyAttack','Hit','Death','Block','Cast')]
        original=load('_v80_world_original_fixture','build_dornfeld_v77.py')
        packages+=['/Game/Askir/V77/Items/'+name for name in original.design_contract()['shard_ids']+['Dornfeld_WellRelic']]
        packages+=['/Game/Askir/V77/Materials/'+name for name in original.MATERIAL_NAMES]
        packages+=['/Game/Askir/V77/Audio/S_'+cue for cue in original.AUDIO_CUES]+['/Game/Askir/V78/Audio/S_SpellCast','/Game/Askir/V78/Audio/S_SpellImpact']
        economy=load('_v80_world_economy_fixture','build_dornfeld_v79.py');econ=economy.source_preflight(ROOT)
        packages+=[econ['asset_root']+'/'+item['id'] for item in econ['items']]
        self.data={'status':'Bestanden','map_package':BUILDER.MAP_PATH,'gameplay_status':'Nicht ausgeführt','maps':{},'reciprocal_routes':{'status':'Bestanden','portal_ids':[entry['portal']['id'] for entry in DATA['maps']]},'startup_map_config':{'status':'Bestanden','game_default_map':BUILDER.MAP_PATH,'editor_startup_map':BUILDER.MAP_PATH,'maps_to_cook':list(BUILDER.MAP_PATHS)}}
        for entry in DATA['maps']:
            result={'status':'Bestanden','map_reload':True,'world_asset_type':'UWorld','native_game_mode':'Bestanden','objective_references':'Bestanden','spawn_collision_traces':'Bestanden','arrival_collision':'Bestanden','delivery_props':'Bestanden','player_capsule_extent':[42,42,96], 'navigation_paths':{'status':'Bestanden','targets':BUILDER.expected_targets(DATA,entry['package'])},'route':copy.deepcopy(entry['portal']),'asset_package_files':packages[:],'animation_sequences':'Bestanden','animation_skeletons':skeletons[:]}
            self.data['maps'][entry['package']]=result
            self.fixture_file(entry['package'],'.umap',1024)
        inherited=self.data['maps'][BUILDER.MAP_PATH]
        inherited.update(relay_source_references='Bestanden',navigation_brush_geometry='Bestanden',audio_asset_references='Bestanden')
        inherited['economy']={'status':'Bestanden','merchant_id':econ['merchant_id'],'recipe_count':len(econ['recipes']),'navigation_targets':[economy.MERCHANT_LABEL,economy.STATION_LABEL]+[entry['id'] for entry in econ['resource_pickups']]}
        for package in packages:self.fixture_file(package,'.uasset',128)
        self.report=self.root/'world.json'

    def tearDown(self): self.directory.cleanup()
    def fixture_file(self,package,suffix,size):
        file=self.root/('Content/'+package[6:]+suffix);file.parent.mkdir(parents=True,exist_ok=True);file.write_bytes(b'fixture'*(size//7+1));return file
    def check(self):
        self.report.write_text(json.dumps(self.data));return RUNNER.check_map_report(self.report,self.root)
    def reject(self,change):
        change()
        with self.assertRaises(RuntimeError):self.check()
    def test_valid_evidence_does_not_claim_runtime_gameplay(self):
        result=self.check();self.assertEqual(result['status'],'Bestanden');self.assertEqual(result['gameplay_status'],'Nicht ausgeführt');self.assertEqual(len(result['map_files']),2)
    def test_single_map_report_rejected(self):self.reject(lambda:self.data['maps'].pop(BUILDER.OUTSKIRTS_PATH))
    def test_missing_actual_second_map_rejected(self):self.reject(lambda:(self.root/'Content/Askir/Maps/Dornfelder_Umland_V80.umap').unlink())
    def test_non_world_reload_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH].update(world_asset_type='Object'))
    def test_missing_capsule_collision_evidence_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH].update(arrival_collision='Nicht ausgeführt'))
    def test_actual_capsule_larger_than_arrival_gap_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH].update(player_capsule_extent=[300,300,400]))
    def test_cargo_navigation_missing_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH]['navigation_paths']['targets'].pop())
    def test_wrong_saved_route_id_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH]['route'].update(destination_id='Other'))
    def test_missing_stock_persistence_identity_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.MAP_PATH]['economy'].update(merchant_id='Other'))
    def test_missing_well_relay_validation_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.MAP_PATH].update(relay_source_references='Nicht ausgeführt'))
    def test_missing_real_reward_asset_rejected(self):self.reject(lambda:(self.root/'Content/Askir/V80/Items/item_002.uasset').unlink())
    def test_missing_real_cargo_prop_rejected(self):self.reject(lambda:(self.root/'Content/Askir/V80/Props/SM_AskirSupplyCrate.uasset').unlink())
    def test_missing_prop_validation_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH].update(delivery_props='Nicht ausgeführt'))
    def test_editor_report_cannot_claim_runtime_playthrough(self):self.reject(lambda:self.data.update(gameplay_status='Bestanden'))
    def test_missing_real_animation_asset_rejected(self):self.reject(lambda:(self.root/'Content/Askir/CharactersV78/Hero/A_Cast.uasset').unlink())
    def test_animation_skeleton_validation_missing_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.OUTSKIRTS_PATH].update(animation_sequences='Nicht ausgeführt'))
    def test_unsafe_package_report_rejected(self):self.reject(lambda:self.data['maps'][BUILDER.MAP_PATH]['asset_package_files'].append('/Game/../outside'))
    def test_single_map_cook_configuration_rejected(self):self.reject(lambda:self.data['startup_map_config']['maps_to_cook'].pop())
    def test_actual_config_cannot_retain_old_startup_map(self):self.reject(lambda:(self.root/'Config/DefaultEngine.ini').write_text('GameDefaultMap=/Game/Askir/Maps/Dornfeld_V79\nEditorStartupMap='+BUILDER.MAP_PATH+'\n'))
    def test_existing_second_world_preserved_before_generation(self):
        unreal=SimpleNamespace(EditorAssetLibrary=SimpleNamespace(does_asset_exist=lambda package:False))
        with self.assertRaisesRegex(RuntimeError,'preserved'):BUILDER.protect_existing_maps(unreal,self.root)
    def test_startup_stays_unchanged_with_one_world_invalid(self):
        config=(self.root/'Config/DefaultEngine.ini').read_text();calls=[]
        village=SimpleNamespace(configure_startup_maps=lambda *args:calls.append(args))
        self.data['maps'][BUILDER.OUTSKIRTS_PATH]['arrival_collision']='Fehlgeschlagen'
        with self.assertRaises(RuntimeError):BUILDER.configure_startup(self.root,village,self.data['maps'])
        self.assertEqual(calls,[]);self.assertEqual(config,(self.root/'Config/DefaultEngine.ini').read_text())
    def test_startup_stays_unchanged_with_non_world_package(self):
        calls=[];village=SimpleNamespace(configure_startup_maps=lambda *args:calls.append(args))
        self.data['maps'][BUILDER.OUTSKIRTS_PATH]['world_asset_type']='Object'
        with self.assertRaises(RuntimeError):BUILDER.configure_startup(self.root,village,self.data['maps'])
        self.assertEqual(calls,[])
    def test_startup_stays_unchanged_without_prop_validation(self):
        calls=[];village=SimpleNamespace(configure_startup_maps=lambda *args:calls.append(args))
        self.data['maps'][BUILDER.OUTSKIRTS_PATH]['delivery_props']='Nicht ausgeführt'
        with self.assertRaises(RuntimeError):BUILDER.configure_startup(self.root,village,self.data['maps'])
        self.assertEqual(calls,[])


class WorldHostBoundaries(unittest.TestCase):
    def test_exact_default_and_explicit_map_readiness(self):
        self.assertEqual(RUNNER.readiness_map('ASKIR_V80_RUNTIME_READY map=Dornfeld_V80'),'Dornfeld_V80')
        self.assertEqual(RUNNER.readiness_map('ASKIR_V80_RUNTIME_READY map=Dornfelder_Umland_V80','Dornfelder_Umland_V80'),'Dornfelder_Umland_V80')
        for wrong in ('Entry','Dornfeld_V79','Dornfeld_V80_Fake'):
            with self.subTest(wrong=wrong),self.assertRaises(RuntimeError):RUNNER.readiness_map('ASKIR_V80_RUNTIME_READY map='+wrong)
        self.assertIsNone(RUNNER.readiness_map('ASKIR_V79_RUNTIME_READY map=Dornfeld_V79'))
    def test_source_preflight_claims_neither_maps_nor_gameplay(self):
        with tempfile.TemporaryDirectory() as directory:
            report=Path(directory)/'source.json'
            result=subprocess.run([sys.executable,str(ROOT/'Tools/build_dornfeld_v80.py'),'--preflight','--report',str(report)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            data=json.loads(report.read_text());self.assertEqual(data['status'],'Bestanden');self.assertEqual(data['editor_status'],'Nicht ausgeführt');self.assertEqual(data['gameplay_status'],'Nicht ausgeführt');self.assertNotIn('map_file',data)
    @unittest.skipIf(importlib.util.find_spec('unreal') is not None,'Needs standalone host without UE Python')
    def test_missing_editor_is_explicitly_blocked_and_produces_no_assets(self):
        before=set((ROOT/'Content').rglob('*.umap'))|set((ROOT/'Content').rglob('*.uasset'))
        with tempfile.TemporaryDirectory() as directory:
            report=Path(directory)/'editor.json'
            result=subprocess.run([sys.executable,str(ROOT/'Tools/build_dornfeld_v80.py'),'--report',str(report)],capture_output=True,text=True)
            self.assertEqual(result.returncode,2,result.stderr);self.assertEqual(json.loads(report.read_text())['status'],'Blockiert')
        self.assertEqual(before,set((ROOT/'Content').rglob('*.umap'))|set((ROOT/'Content').rglob('*.uasset')))


if __name__=='__main__':unittest.main()
