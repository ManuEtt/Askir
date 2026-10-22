"""Validate authored SQ_003 graph connections and canonical runtime action contracts."""
from copy import deepcopy
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("herb_dialogue_catalog_v84", ROOT / "Tools/generate_conversation_catalog_v83.py")
CATALOG = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CATALOG)


class HerbDialogueCatalogTests(unittest.TestCase):
    def setUp(self):
        self.graphs = {g["id"]: g for g in CATALOG.load_catalog(ROOT)}
        self.herbs = self.graphs["D_SQ_003"]
        self.rurik = self.graphs["D_DORNFELD_RURIK"]

    def test_stages_have_explicit_entry_and_choice_specific_completion(self):
        entries = self.herbs["entries"]
        for stage, node in (("AVAILABLE", "n1"), ("EXPLORE", "active"), ("RETURN", "return")):
            self.assertIn((f"SQ_003_STAGE_{stage}", node), [(e["required_flags"][0], e["node"]) for e in entries])
        completions = [e for e in entries if "SQ_003_STAGE_COMPLETED" in e["required_flags"]]
        self.assertEqual([(e["node"], e["required_flags"][1:]) for e in completions],
                         [("thanks_help", ["SQ_003_CHOICE_help"]), ("thanks_bargain", ["SQ_003_CHOICE_bargain"]), ("thanks", [])])

    def test_rurik_greetings_retain_well_lore_and_one_option_for_each_quest_stage(self):
        for node_id, well_choice in (("greeting", "well"), ("remembered", "well_again")):
            node = next(n for n in self.rurik["nodes"] if n["id"] == node_id)
            self.assertIn(well_choice, [c["id"] for c in node["choices"]])
            stage_choices = [c for c in node["choices"] if c["id"].startswith("herbs_")]
            self.assertEqual(len(stage_choices), 4)
            self.assertEqual({tuple(c["required_flags"]) for c in stage_choices},
                             {(f"SQ_003_STAGE_{stage}",) for stage in ("AVAILABLE", "EXPLORE", "RETURN", "COMPLETED")})
            targets = {n["id"] for n in self.rurik["nodes"]}
            self.assertTrue(all(c["next"] in targets for c in stage_choices))

    def test_only_explicit_native_herb_actions_can_mutate_the_new_quest(self):
        for graph in (self.herbs, self.rurik):
            choices = [c for n in graph["nodes"] for c in n["choices"]]
            actions = [(c["action"], c["payload"]) for c in choices if c["action"]]
            self.assertEqual(actions, [("accept_herbs", "help"), ("accept_herbs", "bargain"), ("return_herbs", "")])
            self.assertTrue(all(c["enabled"] for c in choices))
            self.assertTrue(all(not c["granted_flag"].upper().startswith("SQ_") for c in choices))

    def test_herb_action_payload_validation_rejects_malformed_and_conflicting_effects(self):
        for action, payload in (("accept_herbs", "SQ_003"), ("accept_herbs", "pay_everyone"), ("return_herbs", "help")):
            with self.subTest(action=action, payload=payload), self.assertRaises(CATALOG.CatalogError):
                CATALOG.normalize_choice({"text": "Choice", "action": action, "payload": payload}, "node", 1, ["SQ_003"])
        with self.assertRaises(CATALOG.CatalogError):
            CATALOG.normalize_choice({"text": "Choice", "action": "accept_herbs", "payload": "help", "quest_start": "SQ_003"}, "node", 1, ["SQ_003"])

    def test_only_implemented_legacy_quest_start_declarations_are_enabled(self):
        for quest_id, expected in (("SQ_001", True), ("SQ_003", True), ("SQ_002", False), ("SQ_024", False)):
            choice = CATALOG.normalize_choice({"text": "Begin", "quest_start": quest_id}, "node", 1, ["SQ_001", "SQ_002", "SQ_003", "SQ_024"])
            self.assertEqual(choice["enabled"], expected)

    def test_every_herb_graph_edge_points_to_a_real_node(self):
        for graph in (self.herbs, self.rurik):
            nodes = {n["id"] for n in graph["nodes"]}
            self.assertTrue(all(not c["next"] or c["next"] in nodes for n in graph["nodes"] for c in n["choices"]))
            self.assertTrue(all(e["node"] in nodes for e in graph["entries"]))

    def test_mutated_canonical_herb_rewards_are_rejected(self):
        import json
        import shutil
        import tempfile
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in CATALOG.INPUTS:
                target = root / name; target.parent.mkdir(parents=True, exist_ok=True); shutil.copyfile(ROOT / name, target)
            path = root / CATALOG.INPUTS[2]
            quests = json.loads(path.read_text())
            next(q for q in quests if q["id"] == "SQ_003")["rewards"]["gold"] = 999
            path.write_text(json.dumps(quests))
            with self.assertRaisesRegex(CATALOG.CatalogError, "Canonical SQ_003"):
                CATALOG.load_catalog(root)


if __name__ == "__main__":
    unittest.main()
