#!/usr/bin/env python3
"""Compile source dialogue into a bounded native catalog; never mutate the source JSON."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
INPUTS = (
    "Production/Dialogue/branching_dialogues.json",
    "Production/Dialogue/runtime_overrides_v83.json",
    "Production/Quests/sidequests.json",
)
OUTPUT = "Source/Askir/Dialogue/AskirConversationCatalog.generated.inl"
IDENTIFIER = re.compile(r"[A-Za-z][A-Za-z0-9_]{0,95}\Z", re.ASCII)
MAX_GRAPHS, MAX_NODES, MAX_CHOICES, MAX_ENTRIES, MAX_FLAGS = 64, 64, 32, 32, 16


class CatalogError(ValueError):
    """An authored catalog violates the runtime contract."""


def fail(message):
    raise CatalogError(message)


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            fail(f"Duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path):
    try:
        raw = path.read_bytes()
        if len(raw) > 2 * 1024 * 1024:
            fail(f"Input exceeds 2 MiB: {path.name}")
        return json.loads(raw.decode("utf-8"), object_pairs_hook=unique_object,
                          parse_constant=lambda value: fail(f"Invalid JSON number: {value}"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise CatalogError(f"Cannot read valid UTF-8 JSON {path}: {error}") from error


def fields(value, required, optional, context):
    if not isinstance(value, dict):
        fail(f"{context} must be an object")
    missing, unknown = set(required) - value.keys(), value.keys() - set(required) - set(optional)
    if missing or unknown:
        fail(f"{context}: missing fields {sorted(missing)}, unknown fields {sorted(unknown)}")


def identifier(value, context):
    if not isinstance(value, str) or not IDENTIFIER.fullmatch(value) or value.casefold() == "none":
        fail(f"{context}: invalid identifier {value!r}")
    return value


def text(value, context, limit=4096):
    if not isinstance(value, str) or not value.strip() or len(value) > limit:
        fail(f"{context}: text must contain 1..{limit} characters")
    if any((ord(char) < 32 and char not in "\n\r\t") or ord(char) == 127 or 0xD800 <= ord(char) <= 0xDFFF for char in value):
        fail(f"{context}: unsupported control character or Unicode surrogate")
    return value


def sequence(value, context, limit, allow_empty=True):
    if not isinstance(value, list) or len(value) > limit or (not allow_empty and not value):
        fail(f"{context}: expected {'0' if allow_empty else '1'}..{limit} entries")
    return value


def unique_ids(values, context):
    seen = set()
    for value in values:
        folded = value.casefold()
        if folded in seen:
            fail(f"{context}: duplicate identifier {value}")
        seen.add(folded)


def flags(value, context):
    result = [identifier(flag, context) for flag in sequence(value, context, MAX_FLAGS)]
    unique_ids(result, context)
    return result


def condition(raw, context):
    stage = raw.get("delivery_stage", -1)
    if type(stage) is not int or stage < -1 or stage > 4:
        fail(f"{context}: delivery_stage must be -1 or 0..4")
    required = flags(raw.get("required_flags", []), context + " required flags")
    forbidden = flags(raw.get("forbidden_flags", []), context + " forbidden flags")
    if {flag.casefold() for flag in required} & {flag.casefold() for flag in forbidden}:
        fail(f"{context}: flag is both required and forbidden")
    return {"delivery_stage": stage, "required_flags": required, "forbidden_flags": forbidden}


def normalize_choice(raw, node_id, index, quests):
    context = f"{node_id} choice {index}"
    fields(raw, {"text"}, {"id", "next", "quest_start", "required_flags", "forbidden_flags", "granted_flag", "action", "payload", "enabled"}, context)
    result = {"id": identifier(raw.get("id", f"{node_id}_choice_{index}"), context),
              "text": text(raw["text"], context, 256), "next": raw.get("next", "")}
    if result["next"]:
        identifier(result["next"], context + " next")
    elif not isinstance(result["next"], str):
        fail(f"{context}: next must be a node identifier or empty string")
    conditions = condition(raw, context)
    result.update({key: conditions[key] for key in ("required_flags", "forbidden_flags")})
    granted = raw.get("granted_flag", "")
    if granted:
        identifier(granted, context + " granted flag")
        if granted.upper().startswith("SQ_"):
            fail(f"{context}: mission receipt flags cannot be granted by dialogue")
    elif not isinstance(granted, str):
        fail(f"{context}: granted_flag must be an identifier or empty string")
    result["granted_flag"] = granted
    action, payload = raw.get("action", ""), raw.get("payload", "")
    if "quest_start" in raw:
        if "action" in raw or "payload" in raw:
            fail(f"{context}: quest_start cannot be combined with action/payload")
        action, payload = "quest_start", raw["quest_start"]
    if action not in ("", "quest_start", "accept_delivery", "return_delivery", "accept_herbs", "return_herbs"):
        fail(f"{context}: unsupported action {action!r}")
    if not isinstance(payload, str):
        fail(f"{context}: payload must be a string")
    if action == "quest_start":
        identifier(payload, context + " quest")
        if payload not in quests:
            fail(f"{context}: unknown quest {payload}")
    elif action in ("accept_delivery", "accept_herbs"):
        if payload not in ("help", "bargain"):
            fail(f"{context}: invalid mission choice payload")
    elif payload:
        fail(f"{context}: action {action!r} does not accept a payload")
    enabled = raw.get("enabled", True)
    if type(enabled) is not bool:
        fail(f"{context}: enabled must be boolean")
    # Only quests with real native runtime implementations are callable.
    result.update(action=action, payload=payload, enabled=enabled and (action != "quest_start" or payload in ("SQ_001", "SQ_003")))
    return result


def normalize_graph(raw, quests):
    fields(raw, {"id", "npc", "start", "nodes"}, {"entries"}, "conversation")
    graph_id = identifier(raw["id"], "conversation ID")
    speaker = text(raw["npc"], graph_id + " speaker", 128)
    result = {"id": graph_id, "start": identifier(raw["start"], graph_id + " start"), "nodes": [], "entries": []}
    for node in sequence(raw["nodes"], graph_id + " nodes", MAX_NODES, False):
        fields(node, {"id", "text", "choices"}, {"speaker"}, graph_id + " node")
        node_id = identifier(node["id"], graph_id + " node ID")
        choices = [normalize_choice(choice, node_id, index, quests) for index, choice in
                   enumerate(sequence(node["choices"], node_id + " choices", MAX_CHOICES), 1)]
        unique_ids([choice["id"] for choice in choices], node_id + " choices")
        result["nodes"].append({"id": node_id, "speaker": text(node.get("speaker", speaker), node_id + " speaker", 128),
                                "text": text(node["text"], node_id + " text"), "choices": choices})
    unique_ids([node["id"] for node in result["nodes"]], graph_id + " nodes")
    node_ids = {node["id"] for node in result["nodes"]}
    if result["start"] not in node_ids:
        fail(f"{graph_id}: unknown start node")
    for entry in sequence(raw.get("entries", []), graph_id + " entries", MAX_ENTRIES):
        fields(entry, {"node"}, {"delivery_stage", "required_flags", "forbidden_flags"}, graph_id + " entry")
        node_id = identifier(entry["node"], graph_id + " entry node")
        if node_id not in node_ids:
            fail(f"{graph_id}: entry points to unknown node {node_id}")
        result["entries"].append({"node": node_id, **condition(entry, graph_id + " entry")})
    for node in result["nodes"]:
        for choice in node["choices"]:
            if choice["next"] and choice["next"] not in node_ids:
                fail(f"{graph_id}: unknown edge {node['id']} -> {choice['next']}")
    reachable = {result["start"]} | {entry["node"] for entry in result["entries"]}
    while True:
        expanded = reachable | {choice["next"] for node in result["nodes"] if node["id"] in reachable for choice in node["choices"] if choice["next"]}
        if expanded == reachable:
            break
        reachable = expanded
    if reachable != node_ids:
        fail(f"{graph_id}: unreachable nodes {sorted(node_ids - reachable)}")
    if sum(len(node["text"]) + sum(len(choice["text"]) for choice in node["choices"]) for node in result["nodes"]) > 65536:
        fail(f"{graph_id}: dialogue text exceeds 64 Ki characters")
    return result


def load_catalog(root):
    root = Path(root)
    quest_data = sequence(read_json(root / INPUTS[2]), "sidequests", 256, False)
    quests = [identifier(quest.get("id") if isinstance(quest, dict) else None, "quest ID") for quest in quest_data]
    unique_ids(quests, "quests")
    delivery = next((quest for quest in quest_data if quest["id"] == "SQ_001"), None)
    if delivery is None or delivery.get("giver") != "Torben" or delivery.get("rewards") != {"xp": 100, "gold": 16, "item": "item_002"}:
        fail("Canonical SQ_001 giver/rewards must remain Torben, 100 XP, 16 Gold, item_002")
    herbs = next((quest for quest in quest_data if quest["id"] == "SQ_003"), None)
    if herbs is None or herbs.get("giver") != "Rurik" or herbs.get("rewards") != {"xp": 150, "gold": 24, "item": "item_004"}:
        fail("Canonical SQ_003 giver/rewards must remain Rurik, 150 XP, 24 Gold, item_004")
    sources = [normalize_graph(raw, quests) for raw in sequence(read_json(root / INPUTS[0]), "source conversations", MAX_GRAPHS, False)]
    unique_ids([graph["id"] for graph in sources], "source conversations")
    overrides = read_json(root / INPUTS[1])
    fields(overrides, {"schema_version", "overrides", "additional"}, set(), "runtime overrides")
    if type(overrides["schema_version"]) is not int or overrides["schema_version"] != 1:
        fail("Unsupported override schema_version")
    replacements = [normalize_graph(raw, quests) for raw in sequence(overrides["overrides"], "overrides", MAX_GRAPHS)]
    additions = [normalize_graph(raw, quests) for raw in sequence(overrides["additional"], "additional", MAX_GRAPHS)]
    unique_ids([graph["id"] for graph in replacements + additions], "runtime graph declarations")
    source_ids = {graph["id"] for graph in sources}
    if any(graph["id"] not in source_ids for graph in replacements):
        fail("Runtime override must replace an existing source conversation")
    if any(graph["id"].casefold() in {value.casefold() for value in source_ids} for graph in additions):
        fail("Additional conversation duplicates a source conversation")
    replacements = {graph["id"]: graph for graph in replacements}
    result = [replacements.get(graph["id"], graph) for graph in sources] + additions
    if len(result) > MAX_GRAPHS:
        fail("Combined catalog exceeds graph limit")
    unique_ids([graph["id"] for graph in result], "combined catalog")
    return result


def cpp_string(value):
    """ASCII-only wide literals, including non-BMP Unicode without surrogate escapes."""
    parts = []
    escapes = {"\\": "\\\\", '"': '\\"', "\n": "\\n", "\r": "\\r", "\t": "\\t"}
    for char in value:
        code = ord(char)
        if char in escapes:
            parts.append(escapes[char])
        elif code < 32 or code == 127 or 0xD800 <= code <= 0xDFFF:
            fail("Cannot emit unsupported C++ control character or surrogate")
        elif code < 127:
            parts.append(char)
        elif code <= 0xFFFF:
            parts.append(f"\\u{code:04x}")
        else:
            parts.append(f"\\U{code:08x}")
    return 'TEXT("' + "".join(parts) + '")'


def fname(value):
    return f"FName({cpp_string(value)})" if value else "NAME_None"


def names(values):
    return "{" + ",".join(fname(value) for value in values) + "}"


def render(catalog, digest=""):
    lines = ["// Generated by Tools/generate_conversation_catalog_v83.py; do not edit.",
             f"// Source SHA-256: {digest}", "namespace AskirConversationCatalog", "{",
             "inline bool Build(FName ID,FAskirConversationGraph& Out)", "{", "    Out=FAskirConversationGraph();"]
    for graph in catalog:
        lines += [f"    if(ID=={fname(graph['id'])})", "    {", f"        Out.ID={fname(graph['id'])};Out.StartNode={fname(graph['start'])};"]
        for entry in graph["entries"]:
            lines += ["        {", "            FAskirConversationEntry Entry;",
                      f"            Entry.NodeID={fname(entry['node'])};Entry.Condition.DeliveryStage={entry['delivery_stage']};",
                      f"            Entry.Condition.RequiredFlags={names(entry['required_flags'])};Entry.Condition.ForbiddenFlags={names(entry['forbidden_flags'])};",
                      "            Out.Entries.Add(MoveTemp(Entry));", "        }"]
        for node in graph["nodes"]:
            lines += ["        {", "            FAskirConversationNode Node;", f"            Node.NodeID={fname(node['id'])};",
                      f"            Node.Speaker=FText::FromString({cpp_string(node['speaker'])});",
                      f"            Node.Text=FText::FromString({cpp_string(node['text'])});"]
            for choice in node["choices"]:
                lines += ["            {", "                FAskirConversationChoice Choice;",
                          f"                Choice.ChoiceID={fname(choice['id'])};Choice.Label=FText::FromString({cpp_string(choice['text'])});",
                          f"                Choice.NextNode={fname(choice['next'])};Choice.GrantedFlag={fname(choice['granted_flag'])};",
                          f"                Choice.RequiredFlags={names(choice['required_flags'])};Choice.ForbiddenFlags={names(choice['forbidden_flags'])};",
                          f"                Choice.Action={fname(choice['action'])};Choice.Payload={fname(choice['payload'])};Choice.bEnabled={'true' if choice['enabled'] else 'false'};",
                          "                Node.Choices.Add(MoveTemp(Choice));", "            }"]
            lines += ["            Out.Nodes.Add(MoveTemp(Node));", "        }"]
        lines += ["        return true;", "    }"]
    lines += ["    return false;", "}", "}", ""]
    return "\n".join(lines)


def generate(root, check=False):
    root = Path(root)
    catalog = load_catalog(root)
    digest = hashlib.sha256()
    for relative in INPUTS:
        digest.update(relative.encode("ascii") + b"\0" + (root / relative).read_bytes())
    content = render(catalog, digest.hexdigest())
    output = root / OUTPUT
    if check:
        if not output.is_file() or output.read_bytes() != content.encode("ascii"):
            fail("Generated conversation catalog is missing or stale; run the generator without --check")
    else:
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(content.encode("ascii"))
    return catalog


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root", type=Path, default=ROOT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)
    try:
        catalog = generate(args.project_root, args.check)
    except (CatalogError, OSError) as error:
        print(f"Conversation catalog failed: {error}", file=sys.stderr)
        return 1
    disabled = sum(not choice["enabled"] for graph in catalog for node in graph["nodes"] for choice in node["choices"])
    print(f"V83 conversation catalog: {len(catalog)} graphs, {disabled} disabled unimplemented quest choices; {'verified' if args.check else 'generated'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
