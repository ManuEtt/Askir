"""Offline rejection of false V82 engine evidence; these fixtures are not engine runs."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("_v82_targeting_pipeline_test", ROOT / "Tools/run_unreal_v82.py")
RUNNER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RUNNER)


def evidence(map_name="Dornfeld_V80", player="AskirCharacter_0", title_first=False):
    runtime = "LogTemp: Display: ASKIR_V80_RUNTIME_READY map=" + map_name + " player=" + player + "\n"
    title = "LogTemp: Display: ASKIR_V81_FRONTEND_READY map=" + map_name + " view=Title\n"
    targeting = "LogTemp: Display: ASKIR_V82_TARGETING_READY map=" + map_name + " player=" + player + " component=AskirTargetLockComponent\n"
    return runtime + (title + targeting if title_first else targeting + title)


class TargetingEvidenceBoundaries(unittest.TestCase):
    def test_both_real_world_contracts_accept_either_ui_targeting_order(self):
        for package in RUNNER.MAP_PATHS:
            name = package.rsplit("/", 1)[-1]
            for title_first in (False, True):
                with self.subTest(map_name=name, title_first=title_first):
                    self.assertEqual(RUNNER.startup_readiness_map(evidence(name, title_first=title_first), name), name)

    def test_v81_startup_cannot_prove_v82_targeting(self):
        without_targeting = "\n".join(line for line in evidence().splitlines() if "TARGETING_READY" not in line)
        self.assertIsNone(RUNNER.startup_readiness_map(without_targeting))

    def test_targeting_without_current_world_or_title_cannot_prove_startup(self):
        for removed in ("RUNTIME_READY", "FRONTEND_READY"):
            altered = "\n".join(line for line in evidence().splitlines() if removed not in line)
            with self.subTest(removed=removed):
                if removed == "RUNTIME_READY":
                    with self.assertRaises(RuntimeError):
                        RUNNER.startup_readiness_map(altered)
                else:
                    self.assertIsNone(RUNNER.startup_readiness_map(altered))

    def test_wrong_map_targeting_cannot_be_hidden_by_correct_title(self):
        for wrong in ("Entry", "Dornfeld_V82", "Dornfeld_V80_Fake", "Dornfelder_Umland_V80"):
            altered = evidence().replace("TARGETING_READY map=Dornfeld_V80", "TARGETING_READY map=" + wrong)
            with self.subTest(wrong=wrong), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(altered)

    def test_component_type_must_match_exactly(self):
        for component in ("ActorComponent", "AskirTargetLockComponentFake", "None", ""):
            with self.subTest(component=component), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(evidence().replace("component=AskirTargetLockComponent", "component=" + component))

    def test_targeting_must_be_owned_by_the_possessed_player(self):
        for player in ("Enemy_0", "None", "<null>", "null", ""):
            altered = evidence().replace("TARGETING_READY map=Dornfeld_V80 player=AskirCharacter_0", "TARGETING_READY map=Dornfeld_V80 player=" + player)
            with self.subTest(player=player), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(altered)

    def test_missing_runtime_player_identity_cannot_back_a_component_claim(self):
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map(evidence().replace("RUNTIME_READY map=Dornfeld_V80 player=AskirCharacter_0", "RUNTIME_READY map=Dornfeld_V80"))

    def test_partial_or_unrelated_targeting_marker_is_not_readiness(self):
        for marker in ("ASKIR_V82_TARGETING_READY_TEST", "NOT_ASKIR_V82_TARGETING_READY", "ASKIR_V82_TARGETING_READY:"):
            with self.subTest(marker=marker), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(evidence().replace("ASKIR_V82_TARGETING_READY", marker))

    def test_targeting_before_world_restoration_rejected(self):
        lines = evidence().splitlines()
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map("\n".join([lines[1], lines[0], lines[2]]))

    def test_later_world_restoration_invalidates_earlier_targeting(self):
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map(evidence() + "ASKIR_V80_RUNTIME_READY map=Dornfeld_V80 player=AskirCharacter_1\n")

    def test_old_title_cannot_back_a_later_world_generation(self):
        log = evidence() + evidence(player="AskirCharacter_1").replace("LogTemp: Display: ASKIR_V81_FRONTEND_READY map=Dornfeld_V80 view=Title\n", "")
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map(log)

    def test_later_invalid_component_marker_cannot_hide_behind_earlier_success(self):
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map(evidence() + "ASKIR_V82_TARGETING_READY map=Dornfeld_V80 player=Enemy_0 component=AskirTargetLockComponent\n")


class TargetingPipelineBoundaries(unittest.TestCase):
    def pipeline(self, output):
        return RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)

    def test_new_report_keeps_gameplay_and_target_lock_unverified(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            self.assertEqual(pipeline.report["version"], "V82")
            self.assertEqual(pipeline.report["world_contract_version"], "V80")
            self.assertEqual(pipeline.report["maps"], list(RUNNER.MAP_PATHS))
            for key in ("game_acceptance", "frontend_acceptance", "targeting_acceptance"):
                self.assertEqual(pipeline.report[key], "Nicht ausgeführt")
            self.assertTrue(all(phase["status"] == "Nicht ausgeführt" for phase in pipeline.report["phases"].values()))

    def test_missing_engine_does_not_run_build_or_accept_gameplay(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            with patch.object(RUNNER.WORLD, "preflight", return_value={"status": "Blockiert", "issues": ["No UE5.6 engine"]}), \
                 patch.object(pipeline, "backup") as backup:
                self.assertEqual(pipeline.run(), 2)
            backup.assert_not_called()
            report = json.loads((Path(output) / "pipeline.json").read_text())
            self.assertEqual(report["overall_status"], "Blockiert")
            self.assertEqual(report["version"], "V82")
            self.assertEqual(report["targeting_acceptance"], "Nicht ausgeführt")
            self.assertTrue(all(report["phases"][name]["status"] == "Nicht ausgeführt" for name in RUNNER.PHASES[1:]))

    def test_previous_launch_log_or_console_cannot_be_reused(self):
        for suffix in (".log", "_console.log"):
            with self.subTest(suffix=suffix), tempfile.TemporaryDirectory() as output:
                pipeline = self.pipeline(output)
                (Path(output) / ("packaged_startup_Dornfeld_V80" + suffix)).write_text(evidence())
                with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup") as launch:
                    with self.assertRaises(RuntimeError):
                        pipeline._packaged_map_startup("Dornfeld_V80")
                    launch.assert_not_called()

    def test_successful_v81_parent_launch_without_targeting_fails_v82(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            log = Path(output) / "current_launch_fixture.log"
            log.write_text("\n".join(line for line in evidence().splitlines() if "TARGETING_READY" not in line))
            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", return_value={"status": "Bestanden", "log": str(log)}):
                with self.assertRaises(RuntimeError):
                    pipeline._packaged_map_startup("Dornfeld_V80")

    def test_both_maps_need_independent_current_targeting_evidence(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)

            def simulated_launch(expected, override=None):
                log = Path(output) / (expected + "_fixture.log")
                log.write_text(evidence(expected))
                return {"status": "Bestanden", "log": str(log)}

            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", side_effect=simulated_launch) as launch:
                result = pipeline.packaged_startup()
            self.assertEqual(launch.call_count, 2)
            self.assertEqual(set(result["maps"]), set(RUNNER.MAP_PATHS))
            for launch_result in result["maps"].values():
                self.assertTrue(launch_result["frontend_readiness_marker"])
                self.assertTrue(launch_result["targeting_readiness_marker"])
                self.assertIn("target selection/camera/input/combat not tested", launch_result["coverage"])
            self.assertEqual(pipeline.report["targeting_acceptance"], "Nicht ausgeführt")

    def test_outskirts_missing_component_stops_pipeline_after_village_success(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)

            def simulated_launch(expected, override=None):
                log = Path(output) / (expected + "_fixture.log")
                text = evidence(expected)
                if expected == "Dornfelder_Umland_V80":
                    text = "\n".join(line for line in text.splitlines() if "TARGETING_READY" not in line)
                log.write_text(text)
                return {"status": "Bestanden", "log": str(log)}

            with patch.object(RUNNER.WORLD, "preflight", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "backup", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "editor_build", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "map_generation", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "automation", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "windows_package", return_value={"status": "Bestanden"}), \
                 patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", side_effect=simulated_launch):
                self.assertEqual(pipeline.run(), 1)
            self.assertEqual(pipeline.report["overall_status"], "Fehlgeschlagen")
            self.assertEqual(pipeline.report["phases"]["packaged_startup"]["status"], "Fehlgeschlagen")
            self.assertEqual(pipeline.report["game_acceptance"], "Nicht ausgeführt")
            self.assertEqual(pipeline.report["targeting_acceptance"], "Nicht ausgeführt")

    def test_backup_is_distinct_and_cannot_overwrite_an_existing_v82_snapshot(self):
        with tempfile.TemporaryDirectory() as sandbox:
            root = Path(sandbox) / "project"
            root.mkdir()
            project = root / "AskirRPG.uproject"
            project.write_text("{}")
            source = root / "Source"
            source.mkdir()
            (source / "example.cpp").write_text("// source snapshot\n")
            pipeline = RUNNER.Pipeline(project, None, Path(sandbox) / "output")
            result = pipeline.backup()
            destination = Path(result["path"])
            self.assertEqual(destination.name, "Askir_pre_v82_generation")
            self.assertEqual((destination / "Source/example.cpp").read_text(), "// source snapshot\n")
            with self.assertRaises(RuntimeError):
                pipeline.backup()

    def test_backup_inside_project_is_rejected_before_recursive_copy(self):
        with tempfile.TemporaryDirectory() as sandbox:
            project = Path(sandbox) / "AskirRPG.uproject"
            project.write_text("{}")
            pipeline = RUNNER.Pipeline(project, None, Path(sandbox) / "output")
            with self.assertRaises(RuntimeError):
                pipeline.backup()
            self.assertFalse((pipeline.output / "Backup/Askir_pre_v82_generation").exists())


if __name__ == "__main__":
    unittest.main()
