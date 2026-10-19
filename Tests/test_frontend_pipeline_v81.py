"""Offline tests of V81 engine-evidence rejection; no Unreal execution is claimed."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("_v81_frontend_pipeline_test", ROOT / "Tools/run_unreal_v81.py")
RUNNER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RUNNER)


def evidence(map_name="Dornfeld_V80"):
    return ("LogTemp: Display: ASKIR_V80_RUNTIME_READY map=" + map_name + " player=Player_0\n"
            "LogTemp: Display: ASKIR_V81_FRONTEND_READY map=" + map_name + " view=Title\n")


class FrontendEvidenceBoundaries(unittest.TestCase):
    def test_both_preserved_worlds_require_their_own_frontend(self):
        for package in RUNNER.MAP_PATHS:
            name = package.rsplit("/", 1)[-1]
            with self.subTest(map_name=name):
                self.assertEqual(RUNNER.frontend_readiness_map(evidence(name), name), name)

    def test_runtime_readiness_alone_cannot_prove_title_widget(self):
        self.assertIsNone(RUNNER.frontend_readiness_map("ASKIR_V80_RUNTIME_READY map=Dornfeld_V80 player=Player_0"))

    def test_frontend_before_arrival_restoration_rejected(self):
        lines = evidence().splitlines()
        with self.assertRaises(RuntimeError):
            RUNNER.frontend_readiness_map("\n".join(reversed(lines)))

    def test_frontend_without_possessed_world_rejected(self):
        with self.assertRaises(RuntimeError):
            RUNNER.frontend_readiness_map("ASKIR_V81_FRONTEND_READY map=Dornfeld_V80 view=Title")

    def test_other_world_or_title_suffix_rejected(self):
        for wrong in ("Entry", "Dornfeld_V81", "Dornfeld_V80_Fake"):
            with self.subTest(wrong=wrong), self.assertRaises(RuntimeError):
                RUNNER.frontend_readiness_map(evidence().replace("FRONTEND_READY map=Dornfeld_V80", "FRONTEND_READY map=" + wrong))

    def test_incomplete_or_unrelated_menu_marker_rejected(self):
        for altered in ("view=Pause", "view=TitleFake", "", "Title"):
            with self.subTest(altered=altered), self.assertRaises(RuntimeError):
                RUNNER.frontend_readiness_map(evidence().replace("view=Title", altered))

    def test_later_invalid_frontend_marker_cannot_be_hidden_by_an_earlier_valid_one(self):
        with self.assertRaises(RuntimeError):
            RUNNER.frontend_readiness_map(evidence() + "ASKIR_V81_FRONTEND_READY map=Entry view=Title\n")


class FrontendPipelineBoundaries(unittest.TestCase):
    def pipeline(self, output):
        return RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)

    def test_new_report_distinguishes_ui_version_from_real_world_packages(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            self.assertEqual(pipeline.report["version"], "V81")
            self.assertEqual(pipeline.report["world_contract_version"], "V80")
            self.assertEqual(pipeline.report["maps"], list(RUNNER.WORLD.MAP_PATHS))
            self.assertEqual(pipeline.report["game_acceptance"], "Nicht ausgeführt")
            self.assertEqual(pipeline.report["frontend_acceptance"], "Nicht ausgeführt")
            self.assertTrue(all(value["status"] == "Nicht ausgeführt" for value in pipeline.report["phases"].values()))

    def test_environment_block_leaves_generation_build_and_gameplay_unexecuted(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            with patch.object(RUNNER.WORLD, "preflight", return_value={"status": "Blockiert", "issues": ["No engine"]}):
                self.assertEqual(pipeline.run(), 2)
            report = json.loads((Path(output) / "pipeline.json").read_text())
            self.assertEqual(report["overall_status"], "Blockiert")
            self.assertEqual(report["version"], "V81")
            self.assertEqual(report["game_acceptance"], "Nicht ausgeführt")
            self.assertTrue(all(report["phases"][name]["status"] == "Nicht ausgeführt" for name in RUNNER.PHASES[1:]))

    def test_existing_logs_cannot_be_reused_as_startup_evidence(self):
        for suffix in (".log", "_console.log"):
            with self.subTest(suffix=suffix), tempfile.TemporaryDirectory() as output:
                pipeline = self.pipeline(output)
                (Path(output) / ("packaged_startup_Dornfeld_V80" + suffix)).write_text(evidence())
                with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup") as parent:
                    with self.assertRaises(RuntimeError):
                        pipeline._packaged_map_startup("Dornfeld_V80")
                    parent.assert_not_called()

    def test_runtime_only_parent_success_is_rejected(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            log = Path(output) / "current_launch_fixture.log"
            log.write_text(evidence().splitlines()[0])
            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", return_value={"status": "Bestanden", "log": str(log)}):
                with self.assertRaises(RuntimeError):
                    pipeline._packaged_map_startup("Dornfeld_V80")

    def test_current_both_marker_fixture_only_proves_startup_contract(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            log = Path(output) / "current_launch_fixture.log"
            log.write_text(evidence())
            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", return_value={"status": "Bestanden", "log": str(log)}):
                result = pipeline._packaged_map_startup("Dornfeld_V80")
            self.assertTrue(result["frontend_readiness_marker"])
            self.assertIn("visual/input/gameplay not tested", result["coverage"])
            self.assertEqual(pipeline.report["frontend_acceptance"], "Nicht ausgeführt")

    def test_phase_failure_remains_failure_without_a_game_acceptance_claim(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = self.pipeline(output)
            with patch.object(RUNNER.WORLD, "preflight", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "backup", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "editor_build", side_effect=RuntimeError("compiler failed")):
                self.assertEqual(pipeline.run(), 1)
            self.assertEqual(pipeline.report["overall_status"], "Fehlgeschlagen")
            self.assertEqual(pipeline.report["phases"]["editor_build"]["status"], "Fehlgeschlagen")
            self.assertEqual(pipeline.report["phases"]["map_generation"]["status"], "Nicht ausgeführt")
            self.assertEqual(pipeline.report["frontend_acceptance"], "Nicht ausgeführt")


if __name__ == "__main__":
    unittest.main()
