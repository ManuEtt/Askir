"""Behavioral validation of dialogue compilation, hostile inputs and native literal emission."""
from copy import deepcopy
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("conversation_catalog_v83", ROOT / "Tools/generate_conversation_catalog_v83.py")
CATALOG = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CATALOG)


class ConversationCatalogTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        for relative in CATALOG.INPUTS:
            target = self.root / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / relative, target)

    def edit(self, relative, callback):
        path = self.root / relative
        data = json.loads(path.read_text(encoding="utf-8"))
        callback(data)
        path.write_text(json.dumps(data, ensure_ascii=True, indent=2), encoding="utf-8")

    def override(self, callback):
        self.edit(CATALOG.INPUTS[1], lambda data: callback(data["overrides"][0]))

    def reject(self):
        with self.assertRaises(CATALOG.CatalogError):
            CATALOG.load_catalog(self.root)

    def test_complete_catalog_preserves_source_and_disables_unimplemented_quests(self):
        before = {path: (self.root / path).read_bytes() for path in CATALOG.INPUTS}
        graphs = CATALOG.generate(self.root)
        self.assertEqual(len(graphs), 25)
        disabled = [(graph["id"], choice["payload"]) for graph in graphs for node in graph["nodes"] for choice in node["choices"] if not choice["enabled"]]
        self.assertEqual(disabled, [(f"D_SQ_{index:03}", f"SQ_{index:03}") for index in range(2, 25)])
        self.assertEqual(before, {path: (self.root / path).read_bytes() for path in CATALOG.INPUTS})
        CATALOG.generate(self.root, check=True)

    def test_torben_stage_entries_and_persistent_choice_replies(self):
        graph = CATALOG.load_catalog(self.root)[0]
        self.assertEqual(graph["id"], "D_SQ_001")
        self.assertEqual({entry["delivery_stage"] for entry in graph["entries"]}, set(range(5)))
        completions = [entry for entry in graph["entries"] if entry["delivery_stage"] == 4]
        self.assertEqual(completions[0]["required_flags"], ["SQ_001_CHOICE_help"])
        self.assertEqual(completions[1]["required_flags"], ["SQ_001_CHOICE_bargain"])
        actions = [(choice["action"], choice["payload"]) for node in graph["nodes"] for choice in node["choices"] if choice["action"]]
        self.assertEqual(actions, [("accept_delivery", "help"), ("accept_delivery", "bargain"), ("return_delivery", "")])

    def test_rurik_lore_contains_no_quest_or_reward_action(self):
        graph = CATALOG.load_catalog(self.root)[-1]
        self.assertEqual(graph["id"], "D_DORNFELD_RURIK")
        self.assertTrue(5 <= len(graph["nodes"]) <= 8)
        text = " ".join(node["text"] for node in graph["nodes"])
        for canon in ("Ardem", "Falkenwacht", "Grauwald", "fünf Siegel", "Erinnerung", "Mara"):
            self.assertIn(canon, text)
        self.assertTrue(all(not choice["action"] for node in graph["nodes"] for choice in node["choices"]))
        self.assertEqual(graph["entries"][0]["required_flags"], ["RURIK_WELL_LORE"])

    def test_duplicate_source_graph_is_case_insensitive(self):
        def mutate(data):
            duplicate = deepcopy(data[0]);duplicate["id"] = duplicate["id"].lower();data.append(duplicate)
        self.edit(CATALOG.INPUTS[0], mutate);self.reject()

    def test_duplicate_node_is_rejected(self):
        self.override(lambda graph: graph["nodes"].append(deepcopy(graph["nodes"][0])));self.reject()

    def test_duplicate_choice_is_rejected(self):
        self.override(lambda graph: graph["nodes"][0]["choices"].append(deepcopy(graph["nodes"][0]["choices"][0])));self.reject()

    def test_overridden_source_is_still_validated(self):
        self.edit(CATALOG.INPUTS[0], lambda data: data[0]["nodes"][0]["choices"][0].update(next="missing"));self.reject()

    def test_unknown_start_node_is_rejected(self):
        self.override(lambda graph: graph.update(start="missing"));self.reject()

    def test_unknown_entry_node_is_rejected(self):
        self.override(lambda graph: graph["entries"][0].update(node="missing"));self.reject()

    def test_unknown_choice_edge_is_rejected(self):
        self.override(lambda graph: graph["nodes"][0]["choices"][0].update(next="missing"));self.reject()

    def test_unreachable_node_is_rejected(self):
        self.override(lambda graph: graph["nodes"].append({"id": "orphan", "text": "Unused.", "choices": []}));self.reject()

    def test_unknown_action_is_rejected(self):
        self.override(lambda graph: graph["nodes"][0]["choices"][0].update(action="grant_all_gold"));self.reject()

    def test_invalid_delivery_payload_is_rejected(self):
        self.override(lambda graph: graph["nodes"][1]["choices"][0].update(payload="claim_every_reward"));self.reject()

    def test_payload_without_action_is_rejected(self):
        self.override(lambda graph: graph["nodes"][0]["choices"][0].update(payload="SQ_001"));self.reject()

    def test_unknown_quest_reference_is_rejected(self):
        self.edit(CATALOG.INPUTS[0], lambda data: data[1]["nodes"][1]["choices"][0].update(quest_start="SQ_999"));self.reject()

    def test_legacy_and_explicit_actions_cannot_conflict(self):
        self.edit(CATALOG.INPUTS[0], lambda data: data[1]["nodes"][1]["choices"][0].update(action="accept_delivery", payload="help"));self.reject()

    def test_reserved_mission_receipt_cannot_be_granted(self):
        self.override(lambda graph: graph["nodes"][0]["choices"][0].update(granted_flag="SQ_001_CHOICE_help"));self.reject()

    def test_conflicting_required_and_forbidden_flags_are_rejected(self):
        self.override(lambda graph: graph["entries"][0].update(required_flags=["MEMORY"], forbidden_flags=["memory"]));self.reject()

    def test_duplicate_flags_are_rejected(self):
        self.override(lambda graph: graph["entries"][0].update(required_flags=["MEMORY", "memory"]));self.reject()

    def test_bounds_on_nodes_choices_entries_and_flags(self):
        mutations = [
            lambda graph: graph.update(nodes=[graph["nodes"][0]] * 65),
            lambda graph: graph["nodes"][0].update(choices=[graph["nodes"][0]["choices"][0]] * 33),
            lambda graph: graph.update(entries=[graph["entries"][0]] * 33),
            lambda graph: graph["entries"][0].update(required_flags=[f"FLAG_{index}" for index in range(17)]),
        ]
        original = (self.root / CATALOG.INPUTS[1]).read_bytes()
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                (self.root / CATALOG.INPUTS[1]).write_bytes(original)
                self.override(mutation);self.reject()

    def test_invalid_delivery_stage(self):
        for stage in (-2, 5, True, "1"):
            with self.subTest(stage=stage):
                self.override(lambda graph: graph["entries"][0].update(delivery_stage=stage));self.reject()

    def test_invalid_text_values(self):
        for value in ("", " " * 3, "x" * 4097, "bad\x00text", "bad\ud800text", None):
            with self.subTest(value=repr(value)):
                self.override(lambda graph: graph["nodes"][0].update(text=value));self.reject()

    def test_invalid_identifier_is_rejected(self):
        self.override(lambda graph: graph.update(id='bad"); escape(); //'));self.reject()

    def test_choice_label_respects_native_256_character_limit(self):
        self.override(lambda graph: graph["nodes"][0]["choices"][0].update(text="x" * 257));self.reject()

    def test_enabled_must_be_boolean(self):
        self.override(lambda graph: graph["nodes"][0]["choices"][0].update(enabled="yes"));self.reject()

    def test_unknown_fields_are_rejected(self):
        self.override(lambda graph: graph["nodes"][0].update(QuestRewardGold=100));self.reject()

    def test_additional_cannot_shadow_source(self):
        self.edit(CATALOG.INPUTS[1], lambda data: data["additional"][0].update(id="d_sq_001"));self.reject()

    def test_override_must_reference_existing_source(self):
        self.override(lambda graph: graph.update(id="D_MISSING"));self.reject()

    def test_canonical_delivery_reward_cannot_drift(self):
        self.edit(CATALOG.INPUTS[2], lambda data: data[0]["rewards"].update(gold=999));self.reject()

    def test_duplicate_json_keys_are_rejected(self):
        (self.root / CATALOG.INPUTS[1]).write_text('{"schema_version":1,"schema_version":1,"overrides":[],"additional":[]}', encoding="utf-8");self.reject()

    def test_nonfinite_json_number_is_rejected(self):
        (self.root / CATALOG.INPUTS[1]).write_text('{"schema_version":NaN,"overrides":[],"additional":[]}', encoding="utf-8");self.reject()

    def test_invalid_utf8_is_rejected(self):
        (self.root / CATALOG.INPUTS[0]).write_bytes(b"\xff\xfe invalid");self.reject()

    def test_missing_or_stale_generated_catalog_fails_check(self):
        with self.assertRaises(CATALOG.CatalogError):
            CATALOG.generate(self.root, check=True)
        CATALOG.generate(self.root)
        self.override(lambda graph: graph["nodes"][0].update(text="Changed source text."))
        with self.assertRaises(CATALOG.CatalogError):
            CATALOG.generate(self.root, check=True)

    def test_generator_is_deterministic_and_supports_project_root(self):
        command = [sys.executable, str(ROOT / "Tools/generate_conversation_catalog_v83.py"), "--project-root", str(self.root)]
        subprocess.run(command, check=True, capture_output=True, text=True)
        first = (self.root / CATALOG.OUTPUT).read_bytes()
        subprocess.run(command, check=True, capture_output=True, text=True)
        self.assertEqual(first, (self.root / CATALOG.OUTPUT).read_bytes())
        subprocess.run(command + ["--check"], check=True, capture_output=True, text=True)

    def test_unicode_quotes_backslashes_and_newlines_emit_safe_native_literals(self):
        sample = 'Über "Krone" \\ Pfad\n\t☄ 😀'
        self.override(lambda graph: graph["nodes"][0].update(text=sample))
        generated = CATALOG.render(CATALOG.load_catalog(self.root))
        generated.encode("ascii")
        self.assertIn(r'\u00dcber \"Krone\" \\ Pfad\n\t\u2604 \U0001f600', generated)
        self.assertNotIn(r"\ud83d", generated)

    def test_generated_catalog_compiles_with_portable_field_contract(self):
        compiler = shutil.which("g++")
        if compiler is None:
            self.skipTest("Portable C++ compiler unavailable; this is not an Unreal compilation")
        self.override(lambda graph: graph["nodes"][0].update(text='Über "Krone" \\ Pfad\n\t☄ 😀'))
        catalog = CATALOG.load_catalog(self.root)
        source = self.root / "catalog_contract.cpp"
        executable = self.root / "catalog_contract"
        source.write_text(r'''
#include <string>
#include <utility>
#include <vector>
#define TEXT(value) L##value
struct FName {std::wstring Value;FName()=default;FName(const wchar_t* V):Value(V){} bool operator==(const FName& Other)const{return Value==Other.Value;}};
const FName NAME_None;
struct FText {std::wstring Value;static FText FromString(const wchar_t* V){return {V};}};
template<class T> struct TArray:std::vector<T> {using std::vector<T>::operator=;void Add(T Value){this->push_back(std::move(Value));}};
template<class T> T&& MoveTemp(T& Value){return std::move(Value);}
struct FAskirConversationChoice {FName ChoiceID;FText Label;FName NextNode;TArray<FName> RequiredFlags,ForbiddenFlags;FName GrantedFlag,Action,Payload;bool bEnabled=true;};
struct FAskirConversationNode {FName NodeID;FText Speaker,Text;TArray<FAskirConversationChoice> Choices;};
struct FAskirConversationCondition {int DeliveryStage=-1;TArray<FName> RequiredFlags,ForbiddenFlags;};
struct FAskirConversationEntry {FAskirConversationCondition Condition;FName NodeID;};
struct FAskirConversationGraph {FName ID,StartNode;TArray<FAskirConversationEntry> Entries;TArray<FAskirConversationNode> Nodes;};
''' + CATALOG.render(catalog) + r'''
int main(){FAskirConversationGraph Graph;
if(!AskirConversationCatalog::Build(FName(TEXT("D_SQ_001")),Graph)||Graph.Nodes.size()!=11||Graph.Entries.size()!=7)return 1;
if(Graph.Nodes[0].Text.Value!=L"\u00dcber \"Krone\" \\ Pfad\n\t\u2604 \U0001f600")return 5;
if(!AskirConversationCatalog::Build(FName(TEXT("D_DORNFELD_RURIK")),Graph)||Graph.Nodes.size()!=7)return 2;
if(!AskirConversationCatalog::Build(FName(TEXT("D_SQ_024")),Graph)||Graph.Nodes[1].Choices[0].bEnabled)return 3;
if(AskirConversationCatalog::Build(FName(TEXT("UNKNOWN")),Graph)||!Graph.Nodes.empty()||!Graph.Entries.empty())return 4;
return 0;}
''', encoding="ascii")
        subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all", str(source), "-o", str(executable)], check=True, capture_output=True, text=True)
        subprocess.run([str(executable)], check=True, capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
