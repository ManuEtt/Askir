#!/usr/bin/env python3
"""Build V84 SQ_003 on top of the preserved V80 worlds and V81–V83 systems.

Real editor execution adds the herb quest after V83 Rurik validation. Native
component startup markers prove initialization only, never quest playability.
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
import tempfile


def _conversation_runner():
    spec = importlib.util.spec_from_file_location("_askir_v84_conversation_runner", Path(__file__).with_name("run_unreal_v83.py"))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


CONVERSATION = _conversation_runner()
WORLD = CONVERSATION.WORLD
MAP_PATH = CONVERSATION.MAP_PATH
OUTSKIRTS_PATH = CONVERSATION.OUTSKIRTS_PATH
MAP_PATHS = CONVERSATION.MAP_PATHS
PHASES = CONVERSATION.PHASES
REPORT_STATUSES = CONVERSATION.REPORT_STATUSES


def herb_readiness_map(log_text, expected="Dornfeld_V80"):
    lines = log_text.splitlines()
    candidates = [(index, line) for index, line in enumerate(lines) if "ASKIR_V84_HERBS_READY" in line]
    if not candidates:
        return None
    index, line = candidates[-1]
    marker = re.search(r"\bASKIR_V84_HERBS_READY\s+map=([^\s=]+)\s+player=([^\s=]+)\s+component=AskirHerbMissionComponent(?:\s|$)", line)
    if not marker or marker.group(2) in ("None", "null", "<null>"):
        raise RuntimeError("Unvollständiger Kräuter-Bereitschaftsnachweis: " + line)
    if marker.group(1) != expected:
        raise RuntimeError("Kräutermission meldet falsche Map: " + line)
    runtimes = [(i, value) for i, value in enumerate(lines) if "ASKIR_V80_RUNTIME_READY" in value]
    if not runtimes:
        raise RuntimeError("Kräutermission ohne wiederhergestellte Spielerwelt gemeldet")
    runtime_index, runtime_line = runtimes[-1]
    runtime = re.search(r"\bASKIR_V80_RUNTIME_READY\s+map=([^\s=]+)\s+player=([^\s=]+)(?:\s|$)", runtime_line)
    if not runtime or runtime.group(1) != expected or runtime.group(2) != marker.group(2):
        raise RuntimeError("Kräutermission gehört nicht zum zuletzt kontrollierten Spieler")
    if runtime_index >= index:
        raise RuntimeError("Kräutermission wurde vor wiederhergestellter Spielerwelt bereit gemeldet")
    return marker.group(1)


def startup_readiness_map(log_text, expected="Dornfeld_V80"):
    if CONVERSATION.startup_readiness_map(log_text, expected) != expected:
        return None
    return herb_readiness_map(log_text, expected)


def check_inherited_report(data, project_root):
    """Validate the full nested V83 report, including actual on-disk packages."""
    inherited = data.get("inherited_v83")
    if not isinstance(inherited, dict):
        raise RuntimeError("Preserved V83 editor-world validation is missing")
    with tempfile.TemporaryDirectory(prefix="askir-v84-inherited-") as directory:
        report = Path(directory) / "inherited_v83.json"
        report.write_text(json.dumps(inherited), encoding="utf-8")
        result = CONVERSATION.check_conversation_report(report, project_root)
    result["report"] = "inherited_v83 in current V84 editor report"
    return result


def check_herb_report(report_path, project_root):
    report_path, project_root = Path(report_path), Path(project_root)
    if not report_path.is_file():
        raise RuntimeError("Herb editor supplied no current report")
    data = json.loads(report_path.read_text(encoding="utf-8"))
    if data.get("version") != "V84" or data.get("world_contract_version") != "V80" or data.get("status") != "Bestanden" or data.get("editor_status") != "Bestanden" or data.get("gameplay_status") != "Nicht ausgeführt":
        raise RuntimeError("V84 saved-world herb validation was not completed or claims unexecuted gameplay")
    inherited = check_inherited_report(data, project_root)
    builder_spec = importlib.util.spec_from_file_location("_v84_herb_report_builder", project_root / "Tools/build_herb_quest_v84.py")
    builder = importlib.util.module_from_spec(builder_spec)
    builder_spec.loader.exec_module(builder)
    contract = builder.source_preflight(project_root)
    value = data.get("herb_validation", {})
    expected = {"status": "Bestanden", "map_reload": True, "world_asset_type": "UWorld", "map_package": OUTSKIRTS_PATH,
                "native_class": "/Script/Askir.AskirHerbPatchActor", "actor_label": builder.PATCH_LABEL, "stable_tag": builder.PATCH_TAG,
                "source_id": builder.SOURCE_ID, "mesh_package": builder.MESH_PACKAGE, "reward_package": builder.REWARD_PACKAGE,
                "reward_item_id": "item_004", "canonical_reward": "Bestanden", "interaction_query": "Bestanden",
                "visibility_trace": "Bestanden", "walkable_ground": "Bestanden", "capsule_clearance": "Bestanden",
                "approach_clearance": "Bestanden", "navigation_path": "Bestanden", "existing_actors_preserved": "Bestanden"}
    for key, required in expected.items():
        if value.get(key) != required or (key == "map_reload" and value.get(key) is not True):
            raise RuntimeError("Native herb validation missing or incorrect: " + key)
    if type(value.get("unique_actor_count")) is not int or value["unique_actor_count"] != 1:
        raise RuntimeError("Exactly one unique saved herb patch is required")
    if type(value.get("preserved_actor_count")) is not int or value["preserved_actor_count"] <= 0 or type(value.get("created")) is not bool:
        raise RuntimeError("Actual countryside actor preservation evidence missing")
    for key, required in (("position", builder.POSITION), ("visual_offset", builder.VISUAL_OFFSET), ("approach", builder.APPROACH),
                          ("query_offset", builder.QUERY_OFFSET), ("query_extent", builder.QUERY_EXTENT)):
        values = builder.finite_vector(value.get(key), key)
        if any(abs(a - b) > .01 for a, b in zip(values, required)):
            raise RuntimeError("Saved herb geometry differs: " + key)
    extent = builder.finite_vector(value.get("capsule_extent"), "capsule_extent")
    if any(n <= 0 for n in extent) or extent[0] != extent[1] or extent[2] < extent[0]:
        raise RuntimeError("Actual native player capsule dimensions invalid")
    world = json.loads((project_root / "Production/Levels/dornfeld_outskirts_v80.json").read_text(encoding="utf-8"))["maps"][1]
    builder.validate_placement(value["position"], value["approach"], world, contract["navigation_center"], contract["navigation_extent"])
    packages = value.get("asset_package_files")
    required_packages = {builder.MESH_PACKAGE, builder.REWARD_PACKAGE, *contract["material_packages"]}
    if not isinstance(packages, list) or any(not isinstance(package, str) for package in packages) or len(set(packages)) != len(packages) or set(packages) != required_packages:
        raise RuntimeError("Actual herb mesh/reward/material package evidence missing")
    for package in packages:
        builder.package_file(project_root, package)
    giver = data.get("giver_validation", {})
    for key, required in {"status": "Bestanden", "map_reload": True, "map_package": MAP_PATH,
                          "actor_label": "Dornfeld_Rurik_V83", "dialogue_id": "D_DORNFELD_RURIK", "giver_tag": builder.GIVER_TAG,
                          "existing_actors_preserved": "Bestanden"}.items():
        if giver.get(key) != required or (key == "map_reload" and giver.get(key) is not True):
            raise RuntimeError("Trusted saved Rurik giver validation missing: " + key)
    if type(giver.get("unique_actor_count")) is not int or giver["unique_actor_count"] != 1 or type(giver.get("preserved_actor_count")) is not int or giver["preserved_actor_count"] <= 0:
        raise RuntimeError("Trusted unique Rurik identity/preservation evidence missing")
    return {"status": "Bestanden", "report": str(report_path), "preserved_worlds": inherited, "herbs": value,
            "giver": giver, "coverage": "real saved/reloaded herb actor, original asset references, query/visibility/floor/capsule/navigation, trusted Rurik and full inherited worlds; SQ_003 playthrough unexecuted"}


class Pipeline(CONVERSATION.Pipeline):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.report.update(version="V84", world_contract_version="V80", herb_quest_acceptance="Nicht ausgeführt")

    def backup(self):
        destination = self.output / "Backup/Askir_pre_v84_generation"
        if destination.exists():
            raise RuntimeError("Sicherungsziel existiert bereits: " + str(destination))
        if self.output.is_relative_to(self.root):
            raise RuntimeError("Pipeline-Ausgabe/Sicherung muss außerhalb des Projektordners liegen")
        shutil.copytree(self.root, destination, ignore=shutil.ignore_patterns(".git", "Binaries", "Intermediate", "DerivedDataCache", "Saved"))
        return {"status": "Bestanden", "path": str(destination), "coverage": "source/config/content backup before V84 editor mutation; preserved V80 maps"}

    def map_generation(self):
        report = self.output / "herbs_editor.json"
        if report.exists():
            raise RuntimeError("Refusing stale V84 herb world report: " + str(report))
        result = super().map_generation()
        env = dict(os.environ, ASKIR_V84_PROJECT=str(self.root), ASKIR_V84_HERB_REPORT=str(report))
        tool = self.engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
        script = self.root / "Tools/build_herb_quest_v84.py"
        herbs = self.command("herb_quest_generation", [tool, self.project, "-run=pythonscript", "-script=" + str(script),
                             "-unattended", "-nop4", "-nosplash", "-stdout", "-FullStdOutLogOutput"], env=env)
        herbs.update(check_herb_report(report, self.root))
        result["herb_editor"] = herbs
        result["coverage"] = "preserved V80/V83 worlds plus real herb quest actor, original asset/reward/query/visibility/navigation and trusted Rurik marker"
        return result

    def _packaged_map_startup(self, expected, override=None):
        result = super()._packaged_map_startup(expected, override)
        log_text = Path(result["log"]).read_text(encoding="utf-8", errors="replace")
        if startup_readiness_map(log_text, expected) != expected:
            raise RuntimeError("Kein V84-Kräutermission-Bereitschaftsnachweis; " + result["log"])
        result.update(herb_readiness_marker=True,
                      coverage="headless packaged startup: restored player, title, targeting, conversation and herb mission component; quest playthrough untested")
        return result

    def packaged_startup(self):
        result = super().packaged_startup()
        result["coverage"] = "both preserved worlds with V81–V84 component initialization; SQ_003 exploration/reward/travel playthrough untested"
        return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, default=Path(__file__).resolve().parents[1] / "AskirRPG.uproject")
    parser.add_argument("--engine-root", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="New external directory for backup/build/logs")
    parser.add_argument("--timeout", type=int, default=3600)
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--use-existing-map", action="store_true", help="Preserve existing V80 worlds, add missing V83/V84 quest actors and validate existing identities")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    pipeline = Pipeline(args.project, WORLD.discover_engine(args.engine_root), args.output, args.timeout, args.use_existing_map)
    result = pipeline.run(args.check_only)
    print("ASKIR V84:", pipeline.report["overall_status"], "—", pipeline.output / "pipeline.json")
    return result


if __name__ == "__main__":
    sys.exit(main())
