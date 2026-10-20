#!/usr/bin/env python3
"""V82 targeting readiness layered onto V81's build and V80's real world contract.

The preserved runner performs actual editor build, world generation/reload,
registered native tests, Win64 cook/package and separate startup of both worlds.
V82 additionally requires the possessed player's configured targeting component
to announce readiness after world restoration. Log evidence does not exercise
target selection, camera movement, rendering, input or combat.
"""
from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import re
import shutil
import sys


def _frontend_runner():
    spec = importlib.util.spec_from_file_location("_askir_v82_frontend_runner", Path(__file__).with_name("run_unreal_v81.py"))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


FRONTEND = _frontend_runner()
WORLD = FRONTEND.WORLD
MAP_PATH = FRONTEND.MAP_PATH
OUTSKIRTS_PATH = FRONTEND.OUTSKIRTS_PATH
MAP_PATHS = FRONTEND.MAP_PATHS
PHASES = FRONTEND.PHASES
REPORT_STATUSES = FRONTEND.REPORT_STATUSES


def targeting_readiness_map(log_text, expected="Dornfeld_V80"):
    """Bind targeting evidence to the latest restored world and possessed pawn."""
    lines = log_text.splitlines()
    candidates = [(index, line) for index, line in enumerate(lines) if "ASKIR_V82_TARGETING_READY" in line]
    if not candidates:
        return None
    index, line = candidates[-1]
    match = re.search(r"\bASKIR_V82_TARGETING_READY\s+map=([^\s=]+)\s+player=([^\s=]+)\s+component=AskirTargetLockComponent(?:\s|$)", line)
    if not match or match.group(2) in ("None", "null", "<null>"):
        raise RuntimeError("Unvollständiger Zielerfassungs-Bereitschaftsnachweis: " + line)
    if match.group(1) != expected:
        raise RuntimeError("Zielerfassung meldet falsche Map: " + line)
    runtimes = [(position, value) for position, value in enumerate(lines) if "ASKIR_V80_RUNTIME_READY" in value]
    if not runtimes:
        raise RuntimeError("Zielerfassung ohne wiederhergestellte Spielerwelt gemeldet")
    runtime_index, runtime_line = runtimes[-1]
    runtime = re.search(r"\bASKIR_V80_RUNTIME_READY\s+map=([^\s=]+)\s+player=([^\s=]+)(?:\s|$)", runtime_line)
    if not runtime or runtime.group(1) != expected or runtime.group(2) != match.group(2):
        raise RuntimeError("Zielerfassung gehört nicht zum zuletzt kontrollierten Spieler: " + runtime_line)
    if runtime_index >= index:
        raise RuntimeError("Zielerfassung wurde vor wiederhergestellter Spielerwelt bereit gemeldet")
    return match.group(1)


def startup_readiness_map(log_text, expected="Dornfeld_V80"):
    """All three markers must describe the same current startup generation."""
    if FRONTEND.frontend_readiness_map(log_text, expected) != expected:
        return None
    if targeting_readiness_map(log_text, expected) != expected:
        return None
    lines = log_text.splitlines()
    runtime_index = max(index for index, line in enumerate(lines) if "ASKIR_V80_RUNTIME_READY" in line)
    frontend_index = max(index for index, line in enumerate(lines) if "ASKIR_V81_FRONTEND_READY" in line)
    if frontend_index <= runtime_index:
        raise RuntimeError("Titel-Widget gehört nicht zur zuletzt wiederhergestellten Spielerwelt")
    return expected


class Pipeline(FRONTEND.Pipeline):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.report.update(version="V82", world_contract_version="V80",
                           targeting_acceptance="Nicht ausgeführt")

    def backup(self):
        destination = self.output / "Backup/Askir_pre_v82_generation"
        if destination.exists():
            raise RuntimeError("Sicherungsziel existiert bereits: " + str(destination))
        if self.output.is_relative_to(self.root):
            raise RuntimeError("Pipeline-Ausgabe/Sicherung muss außerhalb des Projektordners liegen")
        ignored = shutil.ignore_patterns(".git", "Binaries", "Intermediate", "DerivedDataCache", "Saved")
        shutil.copytree(self.root, destination, ignore=ignored)
        return {"status": "Bestanden", "path": str(destination),
                "coverage": "source/config/content backup before editor mutation; V80 maps preserved"}

    def _packaged_map_startup(self, expected, override=None):
        # V81 rejects stale launch logs and requires world restoration and title.
        result = super()._packaged_map_startup(expected, override)
        log_text = Path(result["log"]).read_text(encoding="utf-8", errors="replace")
        if startup_readiness_map(log_text, expected) != expected:
            raise RuntimeError("Kein V82-Zielerfassungs-Bereitschaftsnachweis; " + result["log"])
        result.update(targeting_readiness_marker=True,
                      coverage="headless packaged startup: restored player, visible title and configured player targeting component; target selection/camera/input/combat not tested")
        return result

    def packaged_startup(self):
        result = super().packaged_startup()
        result["coverage"] = "default village and explicit countryside launch with V81 title and V82 targeting readiness; visual/input/target selection/reciprocal travel not tested"
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
    print("ASKIR V82:", pipeline.report["overall_status"], "—", pipeline.output / "pipeline.json")
    return result


if __name__ == "__main__":
    sys.exit(main())
