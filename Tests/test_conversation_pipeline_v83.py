"""Offline falsified-evidence and preservation boundaries, never actual Unreal runs."""
from __future__ import annotations

import copy
import importlib.util
import itertools
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]


def load(name, relative):
    spec = importlib.util.spec_from_file_location(name, ROOT / relative)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


RUNNER = load("_v83_conversation_pipeline_test", "Tools/run_unreal_v83.py")
BUILDER = load("_v83_conversation_builder_test", "Tools/build_conversations_v83.py")


def evidence(map_name="Dornfeld_V80", player="AskirCharacter_0", order=("title", "target", "conversation")):
    lines = {"runtime": "ASKIR_V80_RUNTIME_READY map=" + map_name + " player=" + player,
             "title": "ASKIR_V81_FRONTEND_READY map=" + map_name + " view=Title",
             "target": "ASKIR_V82_TARGETING_READY map=" + map_name + " player=" + player + " component=AskirTargetLockComponent",
             "conversation": "ASKIR_V83_CONVERSATION_READY map=" + map_name + " player=" + player + " component=AskirConversationComponent"}
    return "\n".join("LogTemp: Display: " + lines[key] for key in ("runtime",) + order) + "\n"


def editor_report():
    return {"version": "V83", "world_contract_version": "V80", "status": "Bestanden", "editor_status": "Bestanden",
            "gameplay_status": "Nicht ausgeführt", "conversation_validation": {
                "status": "Bestanden", "map_reload": True, "world_asset_type": "UWorld", "map_package": RUNNER.MAP_PATH,
                "actor_label": "Dornfeld_Rurik_V83", "stable_tag": "ASKIR_V83_NPC_RURIK", "dialogue_id": "D_DORNFELD_RURIK",
                "native_class": "/Script/Askir.AskirNPC", "npc_name": "Rurik", "unique_actor_count": 1,
                "compiled_dialogue_catalog": "Bestanden", "friendly_npc": "Bestanden", "walkable_ground": "Bestanden",
                "capsule_clearance": "Bestanden", "approach_clearance": "Bestanden", "navigation_path": "Bestanden",
                "character_mesh": BUILDER.GUARD_MESH, "animation_packages": [BUILDER.GUARD_ROOT + "A_" + clip for clip in BUILDER.CLIPS],
                "position": list(BUILDER.RURIK_POSITION), "capsule_extent": [42, 42, 96], "created": True,
                "preserved_actor_count": 80, "existing_actors_preserved": "Bestanden"}}


class ConversationStartupBoundaries(unittest.TestCase):
    def test_both_preserved_worlds_accept_all_orders_after_runtime(self):
        for package in RUNNER.MAP_PATHS:
            name = package.rsplit("/", 1)[-1]
            for order in itertools.permutations(("title", "target", "conversation")):
                with self.subTest(map_name=name, order=order):
                    self.assertEqual(RUNNER.startup_readiness_map(evidence(name, order=order), name), name)

    def test_v82_evidence_cannot_prove_conversation_component(self):
        self.assertIsNone(RUNNER.startup_readiness_map(evidence(order=("title", "target"))))

    def test_conversation_does_not_replace_targeting_or_title(self):
        for order in (("title", "conversation"), ("target", "conversation")):
            with self.subTest(order=order):
                self.assertIsNone(RUNNER.startup_readiness_map(evidence(order=order)))

    def test_wrong_map_cannot_use_other_world_readiness(self):
        for wrong in ("Entry", "Dornfeld_V83", "Dornfeld_V80_Fake", "Dornfelder_Umland_V80"):
            altered = evidence().replace("CONVERSATION_READY map=Dornfeld_V80", "CONVERSATION_READY map=" + wrong)
            with self.subTest(map_name=wrong), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(altered)

    def test_component_identity_must_be_exact(self):
        for wrong in ("ActorComponent", "AskirConversationComponentFake", "None", ""):
            with self.subTest(component=wrong), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(evidence().replace("component=AskirConversationComponent", "component=" + wrong))

    def test_conversation_must_belong_to_possessed_player(self):
        for wrong in ("Enemy_0", "None", "<null>", ""):
            altered = evidence().replace("CONVERSATION_READY map=Dornfeld_V80 player=AskirCharacter_0", "CONVERSATION_READY map=Dornfeld_V80 player=" + wrong)
            with self.subTest(player=wrong), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(altered)

    def test_incomplete_or_unrelated_marker_is_rejected(self):
        for wrong in ("ASKIR_V83_CONVERSATION_READY_TEST", "NOT_ASKIR_V83_CONVERSATION_READY", "ASKIR_V83_CONVERSATION_READY:"):
            with self.subTest(marker=wrong), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(evidence().replace("ASKIR_V83_CONVERSATION_READY", wrong))

    def test_conversation_before_world_restoration_is_rejected(self):
        lines = evidence().splitlines()
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map("\n".join([lines[3], lines[0], lines[1], lines[2]]))

    def test_old_conversation_cannot_back_later_world_generation(self):
        log = evidence() + evidence(player="AskirCharacter_1", order=("title", "target"))
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map(log)

    def test_later_invalid_conversation_marker_cannot_hide_behind_valid_one(self):
        with self.assertRaises(RuntimeError):
            RUNNER.startup_readiness_map(evidence() + "ASKIR_V83_CONVERSATION_READY map=Dornfeld_V80 player=Other_0 component=AskirConversationComponent\n")


class ConversationEditorEvidenceBoundaries(unittest.TestCase):
    def check_fixture(self, value):
        with tempfile.TemporaryDirectory() as output:
            report = Path(output) / "editor_fixture.json"
            report.write_text(json.dumps(value))
            # The isolated test only exercises the added V83 report fields. Every
            # real run still calls the complete inherited UWorld/package checker.
            spec = importlib.util.spec_from_file_location

            def loader(name, path):
                result = spec(name, path)
                original = result.loader.exec_module

                def execute(module):
                    original(module)
                    module.source_preflight = lambda root: {"navigation_center": [0, 450, 150], "navigation_extent": [2350, 3100, 350]}

                result.loader.exec_module = execute
                return result

            with patch.object(RUNNER.WORLD, "check_map_report", return_value={"status": "Bestanden"}) as inherited, \
                 patch.object(RUNNER.importlib.util, "spec_from_file_location", side_effect=loader):
                result = RUNNER.check_conversation_report(report, ROOT)
            inherited.assert_called_once_with(report, ROOT)
            return result

    def test_isolated_valid_fields_keep_gameplay_explicitly_unexecuted(self):
        result = self.check_fixture(editor_report())
        self.assertEqual(result["status"], "Bestanden")
        self.assertIn("dialogue playthrough unexecuted", result["coverage"])

    def test_metadata_alone_does_not_replace_real_map_or_asset_evidence(self):
        with tempfile.TemporaryDirectory() as output:
            report = Path(output) / "metadata_only_fixture.json"
            report.write_text(json.dumps(editor_report()))
            with self.assertRaises(RuntimeError):
                RUNNER.check_conversation_report(report, ROOT)

    def test_every_saved_catalog_collision_navigation_and_preservation_field_is_required(self):
        keys = ("map_reload", "world_asset_type", "map_package", "actor_label", "stable_tag", "dialogue_id", "native_class", "npc_name", "compiled_dialogue_catalog", "friendly_npc", "walkable_ground", "capsule_clearance", "approach_clearance", "navigation_path", "existing_actors_preserved", "character_mesh")
        for key in keys:
            altered = editor_report()
            altered["conversation_validation"].pop(key)
            with self.subTest(missing=key), self.assertRaises(RuntimeError):
                self.check_fixture(altered)

    def test_duplicate_or_boolean_actor_count_is_rejected(self):
        for wrong in (0, 2, True, 1.0, "1"):
            altered = editor_report()
            altered["conversation_validation"]["unique_actor_count"] = wrong
            with self.subTest(count=wrong), self.assertRaises(RuntimeError):
                self.check_fixture(altered)

    def test_invalid_native_capsule_and_nonfinite_positions_are_rejected(self):
        cases = (("capsule_extent", [42, 43, 96]), ("capsule_extent", [42, 42, 20]), ("capsule_extent", [0, 0, 96]),
                 ("position", [True, -1550, 100]), ("position", [float("nan"), -1550, 100]), ("position", [500, float("inf"), 100]),
                 ("position", [500, -1550]), ("position", [5000, -1550, 100]))
        for key, vector in cases:
            altered = editor_report()
            altered["conversation_validation"][key] = vector
            with self.subTest(key=key, vector=vector), self.assertRaises((RuntimeError, ValueError)):
                self.check_fixture(altered)

    def test_real_native_guard_animation_references_cannot_be_replaced_by_partial_list(self):
        for replacement in ([], editor_report()["conversation_validation"]["animation_packages"][:-1], [BUILDER.GUARD_ROOT + "A_Idle"] * 12):
            altered = editor_report()
            altered["conversation_validation"]["animation_packages"] = replacement
            with self.subTest(animation_count=len(replacement)), self.assertRaises(RuntimeError):
                self.check_fixture(altered)

    def test_wrong_version_or_source_only_editor_report_is_rejected(self):
        for field, value in (("version", "V82"), ("world_contract_version", "V83"), ("editor_status", "Nicht ausgeführt")):
            altered = editor_report()
            altered[field] = value
            with self.subTest(field=field), self.assertRaises(RuntimeError):
                self.check_fixture(altered)


class ConversationPlacementAndPreservation(unittest.TestCase):
    def test_embedded_editor_python_checks_catalog_without_launching_a_process(self):
        with patch.object(sys, "executable", "C:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"), \
             patch("subprocess.run", side_effect=AssertionError("Editor preflight must stay in-process")):
            result = BUILDER.source_preflight(ROOT)
        self.assertEqual(result["native_catalog_source"], "Bestanden")
        self.assertEqual(result["editor_status"], "Nicht ausgeführt")

    def test_in_process_preflight_still_rejects_stale_native_catalog(self):
        original = Path.read_bytes

        def stale_catalog(path):
            if path.name == "AskirConversationCatalog.generated.inl":
                return b"// stale generated catalog\n"
            return original(path)

        with patch.object(Path, "read_bytes", stale_catalog), \
             patch("subprocess.run", side_effect=AssertionError("Editor preflight must stay in-process")):
            with self.assertRaisesRegex(ValueError, "missing or stale"):
                BUILDER.source_preflight(ROOT)

    def test_source_placement_reads_preserved_floor_and_actual_authored_navigation_bounds(self):
        spec = json.loads((ROOT / "Production/Levels/dornfeld_outskirts_v80.json").read_text())["maps"][0]
        center, extent = BUILDER.navigation_bounds((ROOT / "Tools/build_dornfeld_v77.py").read_text())
        self.assertEqual(BUILDER.validate_placement(BUILDER.RURIK_POSITION, spec, center, extent), list(BUILDER.RURIK_POSITION))

    def test_source_boundaries_reject_outside_floor_nav_and_invalid_height(self):
        spec = {"ground_center": [0, 500, -50], "ground_extent": [2400, 3200, 50]}
        for point in ((5000, -1550, 100), (500, -2700, 100), (500, -1550, 300), (float("inf"), 0, 100), (True, 0, 100)):
            with self.subTest(point=point), self.assertRaises(ValueError):
                BUILDER.validate_placement(point, spec, [0, 450, 150], [2350, 3100, 350])

    def test_navigation_source_must_not_execute_dynamic_expressions_or_accept_ambiguous_bounds(self):
        valid = "helper.create_navigation_bounds(world, unreal.Vector(0, 450, 150), unreal.Vector(2350, 3100, 350))"
        for source in ("x = 1", valid + "\n" + valid, valid.replace("2350", "dangerous()"), valid.replace("2350", "-1")):
            with self.subTest(source=source), self.assertRaises((ValueError, SyntaxError)):
                BUILDER.navigation_bounds(source)

    def test_saved_actor_preservation_rejects_replacements_moves_and_missing_actors(self):
        before = {"/World.Torben": {"label": "Torben", "class": "/Script/Askir.AskirDeliveryQuestActor", "transform": [0] * 9}}
        BUILDER.require_preserved(before, copy.deepcopy(before))
        for field, replacement in (("label", "Other"), ("class", "/Script/Askir.AskirNPC"), ("transform", [1] * 9)):
            after = copy.deepcopy(before)
            after["/World.Torben"][field] = replacement
            with self.subTest(field=field), self.assertRaises(RuntimeError):
                BUILDER.require_preserved(before, after)
        with self.assertRaises(RuntimeError):
            BUILDER.require_preserved(before, {})

    def test_unrelated_or_conflicting_rurik_actors_are_never_adopted(self):
        class NPC:
            def __init__(self, label, name, conversation="None", tags=()):
                self.label = label
                self.values = {"npc_name": name, "conversation_id": conversation, "tags": tags}
            def get_actor_label(self):
                return self.label
            def get_editor_property(self, key):
                return self.values[key]

        unreal = SimpleNamespace(AskirNPC=NPC)
        valid = NPC(BUILDER.RURIK_LABEL, "Rurik", BUILDER.DIALOGUE_ID, [BUILDER.RURIK_TAG])
        self.assertIs(BUILDER.identify_rurik(unreal, [valid]), valid)
        self.assertIsNone(BUILDER.identify_rurik(unreal, [NPC("Other", "Civilian")]))
        for actors in ([NPC("ExistingRurik", "Rurik")], [valid, valid], [NPC(BUILDER.RURIK_LABEL, "Other")], [NPC("Other", "Rurik", BUILDER.DIALOGUE_ID)]):
            before = [copy.deepcopy(actor.values) for actor in actors]
            with self.subTest(actors=actors), self.assertRaises(RuntimeError):
                BUILDER.identify_rurik(unreal, actors)
            self.assertEqual([actor.values for actor in actors], before)


class ConversationPipelineBoundaries(unittest.TestCase):
    def test_no_engine_leaves_editor_build_dialogue_and_game_acceptance_unexecuted(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)
            with patch.object(RUNNER.WORLD, "preflight", return_value={"status": "Blockiert", "issues": ["No engine"]}), \
                 patch.object(pipeline, "map_generation") as generate:
                self.assertEqual(pipeline.run(), 2)
            generate.assert_not_called()
            self.assertEqual(pipeline.report["version"], "V83")
            self.assertEqual(pipeline.report["world_contract_version"], "V80")
            for key in ("game_acceptance", "frontend_acceptance", "targeting_acceptance", "conversation_acceptance"):
                self.assertEqual(pipeline.report[key], "Nicht ausgeführt")

    def test_stale_conversation_report_is_rejected_before_inherited_world_mutation(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", Path("/fixture-engine"), output)
            (Path(output) / "conversations_editor.json").write_text(json.dumps(editor_report()))
            with patch.object(RUNNER.TARGETING.Pipeline, "map_generation") as generate:
                with self.assertRaises(RuntimeError):
                    pipeline.map_generation()
                generate.assert_not_called()

    def test_zero_exit_editor_without_real_success_report_stops_map_phase(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", Path("/fixture-engine"), output)
            with patch.object(RUNNER.TARGETING.Pipeline, "map_generation", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "command", return_value={"status": "Bestanden", "exit_code": 0}):
                with self.assertRaises(RuntimeError):
                    pipeline.map_generation()

    def test_v82_successful_launch_still_requires_current_conversation_marker(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)
            log = Path(output) / "current_fixture.log"
            log.write_text(evidence(order=("title", "target")))
            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", return_value={"status": "Bestanden", "log": str(log)}):
                with self.assertRaises(RuntimeError):
                    pipeline._packaged_map_startup("Dornfeld_V80")

    def test_both_worlds_need_current_conversation_component_without_gameplay_claim(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)

            def current_launch(expected, override=None):
                log = Path(output) / (expected + "_fixture.log")
                log.write_text(evidence(expected))
                return {"status": "Bestanden", "log": str(log)}

            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup", side_effect=current_launch) as launch:
                result = pipeline.packaged_startup()
            self.assertEqual(launch.call_count, 2)
            self.assertEqual(set(result["maps"]), set(RUNNER.MAP_PATHS))
            for value in result["maps"].values():
                self.assertTrue(value["frontend_readiness_marker"])
                self.assertTrue(value["targeting_readiness_marker"])
                self.assertTrue(value["conversation_readiness_marker"])
            self.assertEqual(pipeline.report["conversation_acceptance"], "Nicht ausgeführt")

    def test_previous_startup_log_cannot_be_reused_as_v83_evidence(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)
            (Path(output) / "packaged_startup_Dornfeld_V80.log").write_text(evidence())
            with patch.object(RUNNER.WORLD.Pipeline, "_packaged_map_startup") as launch:
                with self.assertRaises(RuntimeError):
                    pipeline._packaged_map_startup("Dornfeld_V80")
                launch.assert_not_called()

    def test_v83_backup_is_distinct_and_never_overwrites_existing_snapshot(self):
        with tempfile.TemporaryDirectory() as sandbox:
            root = Path(sandbox) / "project"
            root.mkdir()
            project = root / "AskirRPG.uproject"
            project.write_text("{}")
            pipeline = RUNNER.Pipeline(project, None, Path(sandbox) / "output")
            result = pipeline.backup()
            self.assertEqual(Path(result["path"]).name, "Askir_pre_v83_generation")
            self.assertEqual((Path(result["path"]) / "AskirRPG.uproject").read_text(), "{}")
            with self.assertRaises(RuntimeError):
                pipeline.backup()


if __name__ == "__main__":
    unittest.main()
