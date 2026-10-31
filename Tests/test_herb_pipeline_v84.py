"""Offline V84 evidence/placement boundaries; fixtures never claim Unreal execution."""
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


RUNNER = load("_v84_herb_pipeline_test", "Tools/run_unreal_v84.py")
BUILDER = load("_v84_herb_builder_test", "Tools/build_herb_quest_v84.py")


def evidence(map_name="Dornfeld_V80", player="AskirCharacter_0", order=("title", "target", "conversation", "herbs")):
    lines = {"runtime": f"ASKIR_V80_RUNTIME_READY map={map_name} player={player}",
             "title": f"ASKIR_V81_FRONTEND_READY map={map_name} view=Title",
             "target": f"ASKIR_V82_TARGETING_READY map={map_name} player={player} component=AskirTargetLockComponent",
             "conversation": f"ASKIR_V83_CONVERSATION_READY map={map_name} player={player} component=AskirConversationComponent",
             "herbs": f"ASKIR_V84_HERBS_READY map={map_name} player={player} component=AskirHerbMissionComponent"}
    return "\n".join("LogTemp: Display: " + lines[key] for key in ("runtime",) + order) + "\n"


def report_fixture(materials):
    return {"version": "V84", "world_contract_version": "V80", "status": "Bestanden", "editor_status": "Bestanden",
            "gameplay_status": "Nicht ausgeführt", "inherited_v83": {}, "herb_validation": {
                "status": "Bestanden", "map_reload": True, "world_asset_type": "UWorld", "map_package": BUILDER.OUTSKIRTS_PATH,
                "native_class": "/Script/Askir.AskirHerbPatchActor", "actor_label": BUILDER.PATCH_LABEL, "stable_tag": BUILDER.PATCH_TAG,
                "source_id": BUILDER.SOURCE_ID, "unique_actor_count": 1, "preserved_actor_count": 100, "created": True,
                "existing_actors_preserved": "Bestanden", "position": list(BUILDER.POSITION), "visual_offset": list(BUILDER.VISUAL_OFFSET),
                "approach": list(BUILDER.APPROACH), "query_offset": list(BUILDER.QUERY_OFFSET), "query_extent": list(BUILDER.QUERY_EXTENT),
                "capsule_extent": [42, 42, 96], "mesh_package": BUILDER.MESH_PACKAGE, "reward_package": BUILDER.REWARD_PACKAGE,
                "reward_item_id": "item_004", "canonical_reward": "Bestanden", "interaction_query": "Bestanden",
                "visibility_trace": "Bestanden", "walkable_ground": "Bestanden", "capsule_clearance": "Bestanden",
                "approach_clearance": "Bestanden", "navigation_path": "Bestanden",
                "asset_package_files": [BUILDER.MESH_PACKAGE, BUILDER.REWARD_PACKAGE, *materials]},
            "giver_validation": {"status": "Bestanden", "map_reload": True, "map_package": BUILDER.MAP_PATH,
                "actor_label": "Dornfeld_Rurik_V83", "dialogue_id": "D_DORNFELD_RURIK", "giver_tag": BUILDER.GIVER_TAG,
                "unique_actor_count": 1, "preserved_actor_count": 100, "existing_actors_preserved": "Bestanden"}}


class HerbStartupEvidence(unittest.TestCase):
    def test_both_preserved_maps_require_all_components_for_the_same_current_player(self):
        for package in RUNNER.MAP_PATHS:
            name = package.rsplit("/", 1)[-1]
            for order in itertools.permutations(("title", "target", "conversation", "herbs")):
                with self.subTest(map=name, order=order):
                    self.assertEqual(RUNNER.startup_readiness_map(evidence(name, order=order), name), name)

    def test_v83_success_cannot_prove_herb_initialization(self):
        self.assertIsNone(RUNNER.startup_readiness_map(evidence(order=("title", "target", "conversation"))))

    def test_herbs_do_not_replace_inherited_component_readiness(self):
        for missing in ("title", "target", "conversation"):
            with self.subTest(missing=missing):
                order = tuple(key for key in ("title", "target", "conversation", "herbs") if key != missing)
                self.assertIsNone(RUNNER.startup_readiness_map(evidence(order=order)))

    def test_wrong_map_player_and_component_are_rejected(self):
        for old, new in (("HERBS_READY map=Dornfeld_V80", "HERBS_READY map=Dornfelder_Umland_V80"),
                         ("HERBS_READY map=Dornfeld_V80 player=AskirCharacter_0", "HERBS_READY map=Dornfeld_V80 player=Enemy_0"),
                         ("component=AskirHerbMissionComponent", "component=ActorComponent"),
                         ("component=AskirHerbMissionComponent", "component=AskirHerbMissionComponentFake")):
            with self.subTest(new=new), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(evidence().replace(old, new))

    def test_malformed_or_null_marker_cannot_hide_behind_successful_old_evidence(self):
        for marker in ("NOT_ASKIR_V84_HERBS_READY", "ASKIR_V84_HERBS_READY_FAKE", "ASKIR_V84_HERBS_READY:"):
            with self.subTest(marker=marker), self.assertRaises(RuntimeError):
                RUNNER.startup_readiness_map(evidence().replace("ASKIR_V84_HERBS_READY", marker))
        for player in ("None", "null", "<null>", ""):
            with self.subTest(player=player), self.assertRaises(RuntimeError):
                RUNNER.herb_readiness_map(evidence(player=player))

    def test_readiness_before_restoration_or_from_an_old_possession_is_rejected(self):
        lines = evidence().splitlines()
        with self.assertRaises(RuntimeError):
            RUNNER.herb_readiness_map("\n".join([lines[-1], *lines[:-1]]))
        with self.assertRaises(RuntimeError):
            RUNNER.herb_readiness_map(evidence() + evidence(player="AskirCharacter_1", order=("title", "target", "conversation")))
        with self.assertRaises(RuntimeError):
            RUNNER.herb_readiness_map(lines[-1])


class HerbReportEvidence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.contract = BUILDER.source_preflight(ROOT)

    def isolated_check(self, value):
        """Only added V84 fields are isolated; real code always validates V83 too."""
        original = importlib.util.spec_from_file_location

        def loader(name, path):
            spec = original(name, path)
            if name == "_v84_herb_report_builder":
                execute = spec.loader.exec_module

                def run(module):
                    execute(module)
                    module.source_preflight = lambda root: copy.deepcopy(self.contract)
                    module.package_file = lambda root, package: "fixture package boundary separately tested"

                spec.loader.exec_module = run
            return spec

        with tempfile.TemporaryDirectory() as output:
            report = Path(output) / "fixture_editor_report.json"
            report.write_text(json.dumps(value))
            with patch.object(RUNNER, "check_inherited_report", return_value={"status": "Bestanden"}) as inherited, \
                 patch.object(RUNNER.importlib.util, "spec_from_file_location", side_effect=loader):
                result = RUNNER.check_herb_report(report, ROOT)
            inherited.assert_called_once()
            return result

    def fixture(self):
        return report_fixture(self.contract["material_packages"])

    def test_complete_isolated_fields_keep_actual_gameplay_explicitly_unexecuted(self):
        result = self.isolated_check(self.fixture())
        self.assertEqual(result["status"], "Bestanden")
        self.assertIn("SQ_003 playthrough unexecuted", result["coverage"])

    def test_metadata_without_real_preserved_worlds_cannot_pass(self):
        with tempfile.TemporaryDirectory() as output:
            report = Path(output) / "metadata_only.json"
            report.write_text(json.dumps(self.fixture()))
            with self.assertRaises(RuntimeError):
                RUNNER.check_herb_report(report, ROOT)

    def test_every_collision_navigation_visibility_and_preservation_claim_is_required(self):
        keys = ("status", "map_reload", "world_asset_type", "map_package", "native_class", "actor_label", "stable_tag", "source_id",
                "canonical_reward", "interaction_query", "visibility_trace", "walkable_ground", "capsule_clearance", "approach_clearance",
                "navigation_path", "existing_actors_preserved", "reward_item_id", "mesh_package", "reward_package")
        for key in keys:
            value = self.fixture()
            value["herb_validation"].pop(key)
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                self.isolated_check(value)

    def test_query_geometry_cannot_be_below_ground_or_use_a_tiny_unreachable_box(self):
        for key, vector in (("query_offset", [0, 0, -160]), ("query_extent", [1, 1, 1]), ("visual_offset", [0, 0, 0]),
                            ("position", [1100, 900, 0]), ("approach", [1100, 900, 100]), ("query_extent", [65, True, 45]),
                            ("position", [1100, float("nan"), 100]), ("capsule_extent", [42, 43, 96])):
            value = self.fixture()
            value["herb_validation"][key] = vector
            with self.subTest(key=key, vector=vector), self.assertRaises((RuntimeError, ValueError)):
                self.isolated_check(value)

    def test_unique_counts_creation_and_preservation_require_actual_types(self):
        for key, replacement in (("unique_actor_count", True), ("unique_actor_count", 2), ("unique_actor_count", 1.0),
                                 ("preserved_actor_count", 0), ("preserved_actor_count", True), ("created", 1)):
            value = self.fixture()
            value["herb_validation"][key] = replacement
            with self.subTest(key=key, value=replacement), self.assertRaises(RuntimeError):
                self.isolated_check(value)

    def test_exact_original_material_inventory_is_required(self):
        original = self.fixture()["herb_validation"]["asset_package_files"]
        cases = (original[:-1], original + [original[-1]], original[:2] + ["/Game/Askir/UnknownMaterial"],
                 [*original[:-1], "/Game/Askir/V84/Props/Materials/Unrelated"], [*original[:-1], {}])
        for packages in cases:
            value = self.fixture()
            value["herb_validation"]["asset_package_files"] = packages
            with self.subTest(packages=packages), self.assertRaises(RuntimeError):
                self.isolated_check(value)

    def test_only_the_preserved_unique_rurik_can_be_the_giver(self):
        for key, replacement in (("giver_tag", "ASKIR_FAKE_GIVER"), ("dialogue_id", "D_SQ_003"),
                                 ("actor_label", "AnotherRurik"), ("unique_actor_count", 2), ("map_reload", 1),
                                 ("preserved_actor_count", 0), ("existing_actors_preserved", "Nicht ausgeführt")):
            value = self.fixture()
            value["giver_validation"][key] = replacement
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                self.isolated_check(value)

    def test_wrong_version_and_falsely_passed_gameplay_are_rejected(self):
        for key, replacement in (("version", "V83"), ("world_contract_version", "V84"),
                                 ("editor_status", "Nicht ausgeführt"), ("gameplay_status", "Bestanden")):
            value = self.fixture()
            value[key] = replacement
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                self.isolated_check(value)


class HerbSourceAndPackageBoundaries(unittest.TestCase):
    def test_preflight_runs_inside_embedded_python_without_launching_an_editor(self):
        with patch.object(sys, "executable", "C:/UE_5.6/UnrealEditor-Cmd.exe"), \
             patch("subprocess.run", side_effect=AssertionError("Source preflight must stay in-process")):
            value = BUILDER.source_preflight(ROOT)
        self.assertEqual(value["editor_status"], "Nicht ausgeführt")
        self.assertEqual(value["source_id"], "SQ003_Meadow_V84")
        self.assertEqual(len(value["material_packages"]), 6)

    def test_placement_rejects_floor_nav_height_distance_and_nonfinite_coordinates(self):
        world = json.loads((ROOT / "Production/Levels/dornfeld_outskirts_v80.json").read_text())["maps"][1]
        nav_center, nav_extent = [0, 1600, 150], [3100, 4400, 350]
        self.assertEqual(BUILDER.validate_placement(BUILDER.POSITION, BUILDER.APPROACH, world, nav_center, nav_extent)[0], list(BUILDER.POSITION))
        for position, approach in (((4000, 900, 100), (3800, 900, 100)), ((1100, 900, 0), BUILDER.APPROACH),
                                   (BUILDER.POSITION, BUILDER.POSITION), (BUILDER.POSITION, (500, 900, 100)),
                                   ((float("inf"), 900, 100), BUILDER.APPROACH), ((True, 900, 100), BUILDER.APPROACH)):
            with self.subTest(position=position, approach=approach), self.assertRaises(ValueError):
                BUILDER.validate_placement(position, approach, world, nav_center, nav_extent)

    def test_conflicting_labels_tags_and_duplicate_sources_do_not_adopt_actors(self):
        class NPC:
            def __init__(self, label, source="None", tags=()):
                self.label, self.values = label, {"source_id": source, "tags": tags}
            def get_actor_label(self):
                return self.label
            def get_editor_property(self, key):
                return self.values[key]
        unreal = SimpleNamespace(AskirHerbPatchActor=NPC)
        actor = NPC(BUILDER.PATCH_LABEL, BUILDER.SOURCE_ID, [BUILDER.PATCH_TAG])
        self.assertIs(BUILDER.identify_patch(unreal, [actor]), actor)
        self.assertIsNone(BUILDER.identify_patch(unreal, [NPC("Other")]))
        for actors in ([actor, actor], [NPC(BUILDER.PATCH_LABEL)], [NPC("WrongLabel", BUILDER.SOURCE_ID, [BUILDER.PATCH_TAG])]):
            before = [copy.deepcopy(value.values) for value in actors]
            with self.subTest(actors=actors), self.assertRaises(RuntimeError):
                BUILDER.identify_patch(unreal, actors)
            self.assertEqual([value.values for value in actors], before)

    def test_saved_packages_require_unreal_header_and_safe_local_file(self):
        with tempfile.TemporaryDirectory() as output:
            root = Path(output)
            file = root / "Content/Askir/V84/Items/item_004.uasset"
            file.parent.mkdir(parents=True)
            for content in (b"", bytes(128), b"version https://git-lfs.github.com/spec/v1\n" + bytes(128)):
                file.write_bytes(content)
                with self.subTest(bytes=len(content)), self.assertRaises(RuntimeError):
                    BUILDER.package_file(root, BUILDER.REWARD_PACKAGE)
            file.write_bytes(b"\xc1\x83\x2a\x9e" + bytes(124))
            self.assertEqual(BUILDER.package_file(root, BUILDER.REWARD_PACKAGE), str(file))
            for unsafe in ("/Engine/Foo", "/Game/Askir/../Secret", "/Game/Askir/V84//Fake", "/Game/Askir/V84\\Fake"):
                with self.subTest(path=unsafe), self.assertRaises(RuntimeError):
                    BUILDER.package_file(root, unsafe)
            file.unlink()
            outside = root / "outside.uasset"
            outside.write_bytes(b"\xc1\x83\x2a\x9e" + bytes(124))
            file.symlink_to(outside)
            with self.assertRaises(RuntimeError):
                BUILDER.package_file(root, BUILDER.REWARD_PACKAGE)

    def test_invalid_reward_quantities_or_nonfinite_source_data_fail_preflight(self):
        original = Path.read_text
        raw = json.loads((ROOT / "Production/Items/items.json").read_text())
        for key, replacement in (("stack_max", True), ("weight", float("nan")), ("value", -1), ("name", ""), ("category", "weapon")):
            altered = copy.deepcopy(raw)
            next(item for item in altered if item["id"] == "item_004")[key] = replacement

            def read(path, *args, **kwargs):
                if path == ROOT / "Production/Items/items.json":
                    return json.dumps(altered)
                return original(path, *args, **kwargs)

            with self.subTest(key=key), patch.object(Path, "read_text", read), self.assertRaises(ValueError):
                BUILDER.canonical_reward(ROOT)


class HerbPipelinePhases(unittest.TestCase):
    def test_missing_engine_stops_before_mutation_and_never_accepts_quests(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)
            with patch.object(RUNNER.WORLD, "preflight", return_value={"status": "Blockiert", "issues": ["No engine"]}), \
                 patch.object(pipeline, "map_generation") as generate:
                self.assertEqual(pipeline.run(), 2)
            generate.assert_not_called()
            for key in ("game_acceptance", "frontend_acceptance", "targeting_acceptance", "conversation_acceptance", "herb_quest_acceptance"):
                self.assertEqual(pipeline.report[key], "Nicht ausgeführt")

    def test_stale_editor_report_is_rejected_before_inherited_world_changes(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", Path("/fixture-engine"), output)
            (Path(output) / "herbs_editor.json").write_text("{}")
            with patch.object(RUNNER.CONVERSATION.Pipeline, "map_generation") as inherited, self.assertRaises(RuntimeError):
                pipeline.map_generation()
            inherited.assert_not_called()

    def test_zero_exit_editor_without_current_saved_world_report_is_rejected(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", Path("/fixture-engine"), output)
            with patch.object(RUNNER.CONVERSATION.Pipeline, "map_generation", return_value={"status": "Bestanden"}), \
                 patch.object(pipeline, "command", return_value={"status": "Bestanden", "exit_code": 0}), self.assertRaises(RuntimeError):
                pipeline.map_generation()

    def test_v83_packaged_startup_without_herb_component_is_insufficient(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)
            log = Path(output) / "current_fixture.log"
            log.write_text(evidence(order=("title", "target", "conversation")))
            with patch.object(RUNNER.CONVERSATION.Pipeline, "_packaged_map_startup", return_value={"status": "Bestanden", "log": str(log)}), self.assertRaises(RuntimeError):
                pipeline._packaged_map_startup("Dornfeld_V80")

    def test_current_component_marker_does_not_claim_a_playthrough(self):
        with tempfile.TemporaryDirectory() as output:
            pipeline = RUNNER.Pipeline(ROOT / "AskirRPG.uproject", None, output)
            log = Path(output) / "current_fixture.log"
            log.write_text(evidence())
            with patch.object(RUNNER.CONVERSATION.Pipeline, "_packaged_map_startup", return_value={"status": "Bestanden", "log": str(log)}):
                value = pipeline._packaged_map_startup("Dornfeld_V80")
            self.assertTrue(value["herb_readiness_marker"])
            self.assertEqual(pipeline.report["herb_quest_acceptance"], "Nicht ausgeführt")

    def test_backup_has_its_own_version_and_never_overwrites_existing_snapshot(self):
        with tempfile.TemporaryDirectory() as output:
            root = Path(output) / "project"
            root.mkdir()
            project = root / "AskirRPG.uproject"
            project.write_text("{}")
            pipeline = RUNNER.Pipeline(project, None, Path(output) / "build")
            result = pipeline.backup()
            self.assertEqual(Path(result["path"]).name, "Askir_pre_v84_generation")
            self.assertEqual((Path(result["path"]) / "AskirRPG.uproject").read_text(), "{}")
            with self.assertRaises(RuntimeError):
                pipeline.backup()


if __name__ == "__main__":
    unittest.main()
