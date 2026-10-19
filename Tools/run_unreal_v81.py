#!/usr/bin/env python3
"""V81 frontend build runner using the preserved V80 two-world asset contract.

The inherited runner performs real editor builds, V80 asset generation or reload,
native tests in both worlds, Win64 packaging and headless startup. V81 adds a
separate title-widget readiness requirement; neither marker validates rendering,
mouse/keyboard interaction or a quest playthrough.
"""
from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import re
import shutil
import sys


def _world_runner():
    spec = importlib.util.spec_from_file_location("_askir_v81_preserved_world_runner", Path(__file__).with_name("run_unreal_v80.py"))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


WORLD = _world_runner()
MAP_PATH = WORLD.MAP_PATH
OUTSKIRTS_PATH = WORLD.OUTSKIRTS_PATH
MAP_PATHS = WORLD.MAP_PATHS
PHASES = WORLD.PHASES
REPORT_STATUSES = WORLD.REPORT_STATUSES


def frontend_readiness_map(log_text, expected="Dornfeld_V80"):
    """Require the actual title marker after the world's restoration marker."""
    runtime = WORLD.readiness_map(log_text, expected)
    marker = "ASKIR_V81_FRONTEND_READY"
    lines = log_text.splitlines()
    candidates = [(index, line) for index, line in enumerate(lines) if marker in line]
    if not candidates:
        return None
    index, line = candidates[-1]
    match = re.search(r"\bASKIR_V81_FRONTEND_READY\s+map=([^\s]+)\s+view=Title(?:\s|$)", line)
    if not match:
        raise RuntimeError("Unvollständiger Frontend-Bereitschaftsnachweis: " + line)
    if match.group(1) != expected:
        raise RuntimeError("Frontend meldet falsche Map: " + line)
    if runtime is None or not any("ASKIR_V80_RUNTIME_READY" in value for value in lines[:index]):
        raise RuntimeError("Frontend wurde vor wiederhergestellter Spielerwelt bereit gemeldet")
    return match.group(1)


class Pipeline(WORLD.Pipeline):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.report.update(version="V81", world_contract_version="V80", maps=list(MAP_PATHS),
                           frontend_acceptance="Nicht ausgeführt")

    def backup(self):
        destination = self.output / "Backup/Askir_pre_v81_generation"
        if destination.exists():
            raise RuntimeError("Sicherungsziel existiert bereits: " + str(destination))
        if self.output.is_relative_to(self.root):
            raise RuntimeError("Pipeline-Ausgabe/Sicherung muss außerhalb des Projektordners liegen")
        ignored = shutil.ignore_patterns(".git", "Binaries", "Intermediate", "DerivedDataCache", "Saved")
        shutil.copytree(self.root, destination, ignore=ignored)
        return {"status": "Bestanden", "path": str(destination),
                "coverage": "source/config/content backup before editor mutation; V80 maps preserved"}

    def _packaged_map_startup(self, expected, override=None):
        # Do not accept startup evidence from a previous launch.
        for suffix in (".log", "_console.log"):
            path = self.output / ("packaged_startup_" + expected + suffix)
            if path.exists():
                raise RuntimeError("Alter Startup-Bericht muss getrennt bleiben: " + str(path))
        result = super()._packaged_map_startup(expected, override)
        log_text = Path(result["log"]).read_text(encoding="utf-8", errors="replace")
        if frontend_readiness_map(log_text, expected) != expected:
            raise RuntimeError("Kein V81-Titel-Widget-Bereitschaftsnachweis; " + result["log"])
        result.update(frontend_readiness_marker=True,
                      coverage="headless packaged startup: restored player world and constructed visible title widget; visual/input/gameplay not tested")
        return result

    def packaged_startup(self):
        result = super().packaged_startup()
        result["coverage"] = "default village and explicit countryside launch with V81 title readiness; visual/input/reciprocal travel not tested"
        return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, default=Path(__file__).resolve().parents[1] / "AskirRPG.uproject")
    parser.add_argument("--engine-root", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="New external directory for backup/build/logs")
    parser.add_argument("--timeout", type=int, default=3600, help="Max seconds per engine command")
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--use-existing-map", action="store_true", help="Reload both existing V80 worlds; preserve their content")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    pipeline = Pipeline(args.project, WORLD.discover_engine(args.engine_root), args.output, args.timeout, args.use_existing_map)
    result = pipeline.run(args.check_only)
    print("ASKIR V81:", pipeline.report["overall_status"], "—", pipeline.output / "pipeline.json")
    return result


if __name__ == "__main__":
    sys.exit(main())
