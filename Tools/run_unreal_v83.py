#!/usr/bin/env python3
"""V83 conversations layered onto V82's verified V80 world build contract.

After inherited world generation/reload, a second real editor command adds Rurik
if absent, saves/reloads and validates him, then revalidates both existing worlds.
Packaged startup also requires the possessed player's native conversation marker.
These technical checks never stand in for a dialogue playthrough.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import math
import os
from pathlib import Path
import re
import shutil
import sys


def _targeting_runner():
    spec = importlib.util.spec_from_file_location("_askir_v83_targeting_runner", Path(__file__).with_name("run_unreal_v82.py"))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


TARGETING = _targeting_runner()
WORLD = TARGETING.WORLD
MAP_PATH = TARGETING.MAP_PATH
OUTSKIRTS_PATH = TARGETING.OUTSKIRTS_PATH
MAP_PATHS = TARGETING.MAP_PATHS
PHASES = TARGETING.PHASES
REPORT_STATUSES = TARGETING.REPORT_STATUSES


def conversation_readiness_map(log_text, expected="Dornfeld_V80"):
    """A current native conversation component must belong to the possessed pawn."""
    lines = log_text.splitlines()
    candidates = [(index, line) for index, line in enumerate(lines) if "ASKIR_V83_CONVERSATION_READY" in line]
    if not candidates:
        return None
    index, line = candidates[-1]
    match = re.search(r"\bASKIR_V83_CONVERSATION_READY\s+map=([^\s=]+)\s+player=([^\s=]+)\s+component=AskirConversationComponent(?:\s|$)", line)
    if not match or match.group(2) in ("None", "null", "<null>"):
        raise RuntimeError("Unvollständiger Gesprächs-Bereitschaftsnachweis: " + line)
    if match.group(1) != expected:
        raise RuntimeError("Gesprächssystem meldet falsche Map: " + line)
    runtimes = [(position, value) for position, value in enumerate(lines) if "ASKIR_V80_RUNTIME_READY" in value]
    if not runtimes:
        raise RuntimeError("Gesprächssystem ohne wiederhergestellte Spielerwelt gemeldet")
    runtime_index, runtime_line = runtimes[-1]
    runtime = re.search(r"\bASKIR_V80_RUNTIME_READY\s+map=([^\s=]+)\s+player=([^\s=]+)(?:\s|$)", runtime_line)
    if not runtime or runtime.group(1) != expected or runtime.group(2) != match.group(2):
        raise RuntimeError("Gesprächssystem gehört nicht zum zuletzt kontrollierten Spieler: " + runtime_line)
    if runtime_index >= index:
        raise RuntimeError("Gesprächssystem wurde vor wiederhergestellter Spielerwelt bereit gemeldet")
    return match.group(1)


def startup_readiness_map(log_text, expected="Dornfeld_V80"):
    if TARGETING.startup_readiness_map(log_text, expected) != expected:
        return None
    return conversation_readiness_map(log_text, expected)


def check_conversation_report(report_path, project_root):
    """Retain all inherited physical-world evidence; metadata alone is insufficient."""
    report_path = Path(report_path)
    if not report_path.is_file():
        raise RuntimeError("Conversation editor supplied no report")
    data = json.loads(report_path.read_text(encoding="utf-8"))
    if data.get("version") != "V83" or data.get("world_contract_version") != "V80" or data.get("editor_status") != "Bestanden":
        raise RuntimeError("V83 saved-world conversation validation was not completed")
    inherited = WORLD.check_map_report(report_path, project_root)
    value = data.get("conversation_validation", {})
    expected = {"status": "Bestanden", "map_reload": True, "world_asset_type": "UWorld", "map_package": MAP_PATH,
                "actor_label": "Dornfeld_Rurik_V83", "stable_tag": "ASKIR_V83_NPC_RURIK", "dialogue_id": "D_DORNFELD_RURIK",
                "native_class": "/Script/Askir.AskirNPC", "npc_name": "Rurik", "compiled_dialogue_catalog": "Bestanden",
                "friendly_npc": "Bestanden", "walkable_ground": "Bestanden", "capsule_clearance": "Bestanden",
                "approach_clearance": "Bestanden", "navigation_path": "Bestanden", "existing_actors_preserved": "Bestanden",
                "character_mesh": "/Game/Askir/CharactersV78/Guard/SK_AskirGuard"}
    for key, required in expected.items():
        if value.get(key) != required or (key == "map_reload" and value.get(key) is not True):
            raise RuntimeError("Native Rurik validation missing or incorrect: " + key)
    if type(value.get("unique_actor_count")) is not int or value["unique_actor_count"] != 1:
        raise RuntimeError("Exactly one unique real Rurik actor is required")
    if type(value.get("preserved_actor_count")) is not int or value["preserved_actor_count"] <= 0 or type(value.get("created")) is not bool:
        raise RuntimeError("Actual existing-world actor preservation evidence missing")
    position = value.get("position", [])
    extent = value.get("capsule_extent", [])
    for key, vector in (("position", position), ("capsule_extent", extent)):
        if not isinstance(vector, list) or len(vector) != 3 or any(isinstance(number, bool) or not isinstance(number, (int, float)) or not math.isfinite(number) for number in vector):
            raise RuntimeError("Actual finite Rurik geometry missing: " + key)
    if any(number <= 0 for number in extent) or extent[0] != extent[1] or extent[2] < extent[0]:
        raise RuntimeError("Actual native Rurik capsule dimensions invalid")
    builder = importlib.util.spec_from_file_location("_v83_conversation_report_builder", Path(project_root) / "Tools/build_conversations_v83.py")
    contract = importlib.util.module_from_spec(builder)
    builder.loader.exec_module(contract)
    placement = contract.source_preflight(project_root)
    village = json.loads((Path(project_root) / "Production/Levels/dornfeld_outskirts_v80.json").read_text(encoding="utf-8"))["maps"][0]
    contract.validate_placement(position, village, placement["navigation_center"], placement["navigation_extent"])
    animation_packages = [contract.GUARD_ROOT + "A_" + clip for clip in contract.CLIPS]
    if value.get("animation_packages") != animation_packages:
        raise RuntimeError("Rurik's actual Guard animation references are incomplete")
    return {"status": "Bestanden", "report": str(report_path), "world_validation": inherited,
            "conversation": value, "coverage": "real saved/reloaded NPC/catalog/visual/floor/capsule/navigation and preserved two-map editor contract; dialogue playthrough unexecuted"}


class Pipeline(TARGETING.Pipeline):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.report.update(version="V83", world_contract_version="V80", conversation_acceptance="Nicht ausgeführt")

    def backup(self):
        destination = self.output / "Backup/Askir_pre_v83_generation"
        if destination.exists():
            raise RuntimeError("Sicherungsziel existiert bereits: " + str(destination))
        if self.output.is_relative_to(self.root):
            raise RuntimeError("Pipeline-Ausgabe/Sicherung muss außerhalb des Projektordners liegen")
        ignored = shutil.ignore_patterns(".git", "Binaries", "Intermediate", "DerivedDataCache", "Saved")
        shutil.copytree(self.root, destination, ignore=ignored)
        return {"status": "Bestanden", "path": str(destination), "coverage": "source/config/content backup before editor mutation; V80 maps preserved"}

    def map_generation(self):
        report = self.output / "conversations_editor.json"
        if report.exists():
            raise RuntimeError("Refusing stale V83 conversation world report: " + str(report))
        result = super().map_generation()
        env = dict(os.environ, ASKIR_V83_PROJECT=str(self.root), ASKIR_V83_CONVERSATION_REPORT=str(report))
        tool = self.engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
        script = self.root / "Tools/build_conversations_v83.py"
        conversation = self.command("conversation_generation", [tool, self.project, "-run=pythonscript", "-script=" + str(script),
                                      "-unattended", "-nop4", "-nosplash", "-stdout", "-FullStdOutLogOutput"], env=env)
        conversation.update(check_conversation_report(report, self.root))
        result["conversation_editor"] = conversation
        result["coverage"] = "preserved V80 worlds plus real Rurik addition, reload, native catalog/visual/collision/navigation checks and postinsertion two-world revalidation"
        return result

    def _packaged_map_startup(self, expected, override=None):
        result = super()._packaged_map_startup(expected, override)
        log_text = Path(result["log"]).read_text(encoding="utf-8", errors="replace")
        if startup_readiness_map(log_text, expected) != expected:
            raise RuntimeError("Kein V83-Gesprächs-Bereitschaftsnachweis; " + result["log"])
        result.update(conversation_readiness_marker=True,
                      coverage="headless packaged startup: restored player, title, targeting and native conversation component; dialogue/input/camera/combat not tested")
        return result

    def packaged_startup(self):
        result = super().packaged_startup()
        result["coverage"] = "default village and explicit countryside launch with title/targeting/conversation readiness; interaction/dialogue/reciprocal travel not tested"
        return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, default=Path(__file__).resolve().parents[1] / "AskirRPG.uproject")
    parser.add_argument("--engine-root", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="New external directory for backup/build/logs")
    parser.add_argument("--timeout", type=int, default=3600)
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--use-existing-map", action="store_true", help="Preserve V80 world actors and add Rurik only if absent; validate existing V83 Rurik")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    pipeline = Pipeline(args.project, WORLD.discover_engine(args.engine_root), args.output, args.timeout, args.use_existing_map)
    result = pipeline.run(args.check_only)
    print("ASKIR V83:", pipeline.report["overall_status"], "—", pipeline.output / "pipeline.json")
    return result


if __name__ == "__main__":
    sys.exit(main())
