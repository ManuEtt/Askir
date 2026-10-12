"""Offline failure-path tests for the real V78 skeletal importer; no engine claims."""
import importlib.util
import math
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("askir_animation_import_v78", ROOT / "Tools/import_animated_assets_v78.py")
IMPORTER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(IMPORTER)


class Sequence:
    def __init__(self, skeleton, length):
        self.skeleton, self.length = skeleton, length

    def get_editor_property(self, key):
        if key != "skeleton":
            raise KeyError(key)
        return self.skeleton

    def get_play_length(self):
        return self.length


class Properties:
    def __init__(self):
        self.values = {}

    def set_editor_property(self, key, value):
        self.values[key] = value

    def get_editor_property(self, key):
        return self.values[key]


class ImportOptions(Properties):
    def __init__(self):
        super().__init__()
        self.values["skeletal_mesh_import_data"] = Properties()
        self.values["anim_sequence_import_data"] = Properties()


class ImportTask(Properties):
    def __init__(self):
        super().__init__()
        self.values["imported_object_paths"] = []


class AnimationImportTests(unittest.TestCase):
    def fake_unreal(self):
        tasks = []
        engine = SimpleNamespace(
            AssetImportTask=ImportTask,
            FbxImportUI=ImportOptions,
            FBXImportType=SimpleNamespace(FBXIT_SKELETAL_MESH="mesh", FBXIT_ANIMATION="animation"),
            FBXAnimationLengthImportType=SimpleNamespace(FBXALIT_EXPORTED_TIME="exported"),
            AssetToolsHelpers=SimpleNamespace(get_asset_tools=lambda: SimpleNamespace(import_asset_tasks=tasks.extend)),
        )
        return engine, tasks

    def test_animation_import_uses_exact_mesh_skeleton_and_never_imports_a_second_mesh(self):
        engine, tasks = self.fake_unreal()
        skeleton = object()
        IMPORTER._import_task(engine, Path("Cast.fbx"), "Hero", "A_Cast", skeleton)
        self.assertEqual(len(tasks), 1)
        task = tasks[0]
        options = task.values["options"].values
        self.assertIs(options["skeleton"], skeleton)
        self.assertFalse(options["import_mesh"])
        self.assertTrue(options["import_animations"])
        self.assertFalse(options["automated_import_should_detect_type"])
        self.assertFalse(task.values["replace_existing"])
        self.assertEqual(task.values["destination_name"], "A_Cast")
        self.assertEqual(options["mesh_type_to_import"], "animation")
        self.assertEqual(options["anim_sequence_import_data"].values["custom_sample_rate"], 30)

    def test_mesh_import_does_not_auto_create_unknown_animation_packages(self):
        engine, tasks = self.fake_unreal()
        IMPORTER._import_task(engine, Path("Askir_Guard.fbx"), "Guard", "SK_AskirGuard")
        options = tasks[0].values["options"].values
        self.assertTrue(options["import_mesh"])
        self.assertFalse(options["import_animations"])
        self.assertFalse(options["skeletal_mesh_import_data"].values["update_skeleton_reference_pose"])
        self.assertEqual(options["mesh_type_to_import"], "mesh")

    def test_clips_require_same_skeleton_object(self):
        skeleton = object()
        self.assertEqual(IMPORTER.validate_sequence(Sequence(skeleton, 0.85), skeleton, "Hit"), 0.85)
        with self.assertRaisesRegex(RuntimeError, "skeleton mismatch"):
            IMPORTER.validate_sequence(Sequence(object(), 0.85), skeleton, "Hit")

    def test_empty_corrupt_duration_is_rejected(self):
        skeleton = object()
        for value in (0, -0.1, math.nan, math.inf):
            with self.subTest(value=value), self.assertRaisesRegex(RuntimeError, "positive playback duration"):
                IMPORTER.validate_sequence(Sequence(skeleton, value), skeleton, "Death")

    def test_all_sources_are_required_before_import_mutates_assets(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            for source in IMPORTER.fbx_sources(root).values():
                for path in [source["mesh"], *source["animations"].values()]:
                    path.parent.mkdir(parents=True, exist_ok=True)
                    # Deliberately a source-header fixture, not a valid animated mesh.
                    path.write_bytes(b"Kaydara FBX Binary  \x00\x1a\x00" + bytes(150))
            result = IMPORTER.source_preflight(root)
            self.assertEqual(len(result["roles"]), 3)
            self.assertEqual(result["runtime_animation"], "Nicht ausgeführt")
            missing = IMPORTER.fbx_sources(root)["Raider"]["animations"]["Death"]
            missing.unlink()
            with self.assertRaisesRegex(RuntimeError, "Required nonempty FBX source missing"):
                IMPORTER.source_preflight(root)

    def test_fake_success_without_real_package_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaisesRegex(RuntimeError, "actual .uasset is missing"):
                IMPORTER.assert_package(folder, IMPORTER.object_path("Hero", "A_Death"))

    def test_import_paths_cannot_escape_project_character_directory(self):
        for role, name in (("../../Other", "Hero"), ("Hero", "../Bad"), ("Guard", "Name.Name")):
            with self.subTest(role=role, name=name), self.assertRaises(ValueError):
                IMPORTER.object_path(role, name)
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaisesRegex(RuntimeError, "Refusing non-character"):
                IMPORTER.assert_package(folder, "/Game/Elsewhere/Asset.Asset")


if __name__ == "__main__":
    unittest.main()
