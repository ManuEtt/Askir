"""Offline runner checks. These tests cannot compile UE or establish gameplay success."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def load_tool(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / "Tools" / (name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


runner = load_tool("run_unreal_v77")
builder = load_tool("build_dornfeld_v77")


class PipelineEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="askir-pipeline-test-")
        self.root = Path(self.temp.name)

    def tearDown(self):
        self.temp.cleanup()

    def write_report(self, data):
        runner.write_json(self.root / "index.json", data)

    def test_no_report_is_not_success(self):
        with self.assertRaisesRegex(RuntimeError, "keinen index.json"):
            runner.automation_result(self.root, {"Askir.Mission.Test"})

    def test_zero_tests_is_not_success(self):
        self.write_report({"tests": [], "failed": 0})
        with self.assertRaisesRegex(RuntimeError, "keine ausgeführten"):
            runner.automation_result(self.root, {"Askir.Mission.Test"})

    def test_a_missing_required_test_is_not_success(self):
        self.write_report({"tests": [{"testFullPath": "Askir.Other", "state": "Success"}]})
        with self.assertRaisesRegex(RuntimeError, "erforderliche Tests nicht aus"):
            runner.automation_result(self.root, {"Askir.Mission.Test"})

    def test_failed_or_not_run_test_is_not_success(self):
        for state in ("Fail", "NotRun", "InProcess", "Skipped"):
            with self.subTest(state=state):
                self.write_report({"tests": [{"testFullPath": "Askir.Mission.Test", "state": state}]})
                with self.assertRaisesRegex(RuntimeError, "nicht vollständig bestanden"):
                    runner.automation_result(self.root, {"Askir.Mission.Test"})

    def test_nested_success_report_is_accepted_with_coverage(self):
        self.write_report({"tests": [{"children": [{"testFullPath": "Askir.Mission.Test", "state": "Success"}]}],
                           "failed": 0, "notRun": 0, "inProcess": 0})
        result = runner.automation_result(self.root, {"Askir.Mission.Test"})
        self.assertEqual(result["status"], "Bestanden")
        self.assertEqual(result["test_count"], 1)
        self.assertIn("does not imply", result["coverage"])

    def test_success_map_json_without_umap_is_rejected(self):
        report = self.root / "map.json"
        runner.write_json(report, {"status": "Bestanden", "map_package": runner.MAP_PATH,
                                   "validation": {"map_reload": True}})
        with self.assertRaisesRegex(RuntimeError, ".umap fehlt"):
            runner.check_map_report(report, self.root)

    def test_failed_generation_report_is_rejected(self):
        report = self.root / "map.json"
        runner.write_json(report, {"status": "Fehlgeschlagen", "map_package": runner.MAP_PATH, "error": "native class missing"})
        with self.assertRaisesRegex(RuntimeError, "native class missing"):
            runner.check_map_report(report, self.root)

    def test_missing_engine_blocks_and_does_not_generate_assets(self):
        project = self.root / "Project/AskirRPG.uproject"
        project.parent.mkdir()
        project.write_text(json.dumps({"EngineAssociation": "5.6", "Plugins": [
            {"Name": "PythonScriptPlugin", "Enabled": True}, {"Name": "EditorScriptingUtilities", "Enabled": True}]}))
        pipeline = runner.Pipeline(project, None, self.root / "Output")
        self.assertEqual(pipeline.run(), 2)
        self.assertEqual(pipeline.report["overall_status"], "Blockiert")
        self.assertEqual(pipeline.report["phases"]["map_generation"]["status"], "Nicht ausgeführt")
        self.assertFalse((project.parent / "Content").exists())
        self.assertTrue((self.root / "Output/pipeline.json").is_file())

    def test_wrong_engine_version_is_rejected(self):
        project = self.root / "AskirRPG.uproject"
        project.write_text(json.dumps({"EngineAssociation": "5.6"}))
        engine = self.root / "UE57"
        runner.write_json(engine / "Engine/Build/Build.version", {"MajorVersion": 5, "MinorVersion": 7})
        result = runner.preflight(project, engine, "Windows")
        self.assertEqual(result["status"], "Blockiert")
        self.assertIn("Engine ist nicht Unreal 5.6", result["issues"])

    def test_backup_inside_project_is_rejected(self):
        project = self.root / "Project/AskirRPG.uproject"
        project.parent.mkdir()
        project.write_text("{}")
        pipeline = runner.Pipeline(project, None, project.parent / "Output")
        with self.assertRaisesRegex(RuntimeError, "außerhalb des Projektordners"):
            pipeline.backup()

    def test_windows_batch_rejects_command_expansion(self):
        pipeline = runner.Pipeline(self.root / "AskirRPG.uproject", None, self.root / "Output")
        for bad in ("C:/%SECRET%/Build.bat", "C:/build&exit.bat", "C:/evil\nname.bat"):
            with self.subTest(path=bad), self.assertRaisesRegex(RuntimeError, "nicht sicher"):
                pipeline.batch(Path(bad), ["AskirEditor", "Win64"])

    def test_registered_native_tests_are_discovered(self):
        names = runner.required_test_names(ROOT)
        self.assertTrue(names)
        self.assertTrue(all(name.startswith("Askir.") for name in names))

    def test_world_contract_contains_unique_real_sources(self):
        contract = builder.design_contract()
        self.assertEqual(contract["map_package"], runner.MAP_PATH)
        self.assertEqual(len(contract["shard_ids"]), 3)
        self.assertEqual(len(contract["guardian_ids"]), 2)
        ids = contract["shard_ids"] + contract["guardian_ids"] + [contract["chest_id"]]
        self.assertEqual(len(ids), len(set(ids)))

    def test_startup_config_update_preserves_unreal_arrays_and_other_sections(self):
        original = ("[/Script/EngineSettings.GameMapsSettings]\r\n"
                    "GameDefaultMap=/Engine/Maps/Entry\r\n"
                    "+GameModeClassAliases=(Name=One)\r\n"
                    "+GameModeClassAliases=(Name=Two)\r\n"
                    "[/Script/Other.Settings]\r\nOtherMap=/Game/KeepMe\r\n")
        result = builder.startup_config_text(original)
        self.assertIn("GameDefaultMap=" + builder.MAP_PATH + "\r\n", result)
        self.assertIn("EditorStartupMap=" + builder.MAP_PATH + "\r\n", result)
        self.assertIn("+GameModeClassAliases=(Name=One)\r\n+GameModeClassAliases=(Name=Two)", result)
        self.assertIn("[/Script/Other.Settings]\r\nOtherMap=/Game/KeepMe", result)
        self.assertEqual(result.count("GameDefaultMap="), 1)

    def test_startup_config_appends_missing_section(self):
        result = builder.startup_config_text("[AnotherSection]\nSetting=Value")
        self.assertIn("Setting=Value\n", result)
        self.assertIn("[/Script/EngineSettings.GameMapsSettings]\nGameDefaultMap=" + builder.MAP_PATH, result)

    def test_failed_reload_cannot_change_default_map(self):
        config = self.root / "Config/DefaultEngine.ini"
        config.parent.mkdir()
        original = "[/Script/EngineSettings.GameMapsSettings]\nGameDefaultMap=/Engine/Maps/Entry\n"
        config.write_text(original)
        with self.assertRaisesRegex(RuntimeError, "successful saved-map reload"):
            builder.configure_startup_maps(self.root, {"map_reload": False})
        self.assertEqual(config.read_text(), original)

    def test_missing_real_map_cannot_change_default_map_even_with_validation_claim(self):
        config = self.root / "Config/DefaultEngine.ini"
        config.parent.mkdir()
        original = "[/Script/EngineSettings.GameMapsSettings]\nGameDefaultMap=/Engine/Maps/Entry\n"
        config.write_text(original)
        validation = {"map_reload": True, "spawn_collision_traces": "Bestanden", "relay_source_references": "Bestanden",
                      "navigation_brush_geometry": "Bestanden", "native_game_mode": "Bestanden"}
        with self.assertRaisesRegex(RuntimeError, "existing real .umap"):
            builder.configure_startup_maps(self.root, validation)
        self.assertEqual(config.read_text(), original)

    def test_runtime_readiness_requires_exact_default_map(self):
        self.assertIsNone(runner.readiness_map("engine initializing"))
        self.assertEqual(runner.readiness_map("LogTemp: ASKIR_V77_RUNTIME_READY map=Dornfeld_V77 player=Hero"), "Dornfeld_V77")
        for wrong in ("Entry", "Dornfeld_V77_Uncooked", "/Game/Askir/Other"):
            with self.subTest(map=wrong), self.assertRaisesRegex(RuntimeError, "falsche Default-Map"):
                runner.readiness_map("ASKIR_V77_RUNTIME_READY map=" + wrong + " player=Hero")


if __name__ == "__main__":
    unittest.main()
