#!/usr/bin/env python3
"""Run available production-rule tests and record engine blockers separately."""
from pathlib import Path
from datetime import datetime, timezone
import argparse
import json
import platform
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(command):
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=120)
    return {"status": "Bestanden" if result.returncode == 0 else "Fehlgeschlagen",
            "command": command, "exit_code": result.returncode,
            "output": (result.stdout + result.stderr)[-20000:]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, default=ROOT / "Reports/V77_verification.json")
    args = parser.parse_args()
    checks = {"source_package": run([sys.executable, "Tools/validate_release.py"]),
              "python_regressions": run([sys.executable, "-m", "unittest", "discover", "-s", "Tests", "-q"])}
    match = re.search(r"Ran (\d+) tests", checks["python_regressions"]["output"])
    checks["python_regressions"]["test_count"] = int(match.group(1)) if match else None
    compiler = shutil.which("g++")
    for name, source in (("mission_rules", "Tests/cpp/test_dornfeld_rules_v77.cpp"),
                         ("checked_math", "Tests/cpp/test_checked_math_v77.cpp")):
        if not compiler:
            checks[name] = {"status": "Blockiert", "reason": "g++ unavailable"}
            continue
        with tempfile.TemporaryDirectory(prefix="askir-v77-") as directory:
            binary = str(Path(directory) / name)
            compile_result = run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
                                  "-fsanitize=undefined", "-fno-sanitize-recover=all", "-I.", source, "-o", binary])
            checks[name] = {"status": compile_result["status"], "compile": compile_result}
            if compile_result["status"] == "Bestanden":
                checks[name]["execution"] = run([binary])
                checks[name]["status"] = checks[name]["execution"]["status"]
    from run_unreal_v77 import preflight, discover_engine
    engine = preflight(ROOT / "AskirRPG.uproject", discover_engine())
    tests = re.findall(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\s*\([^,]+,\s*"([^"]+)"',
                       "\n".join(path.read_text(encoding="utf-8") for path in (ROOT / "Source/Askir/Tests").glob("*.cpp")))
    report = {"version": "V77", "checked_at": datetime.now(timezone.utc).isoformat(),
              "platform": platform.platform(), "checks": checks,
              "scope": "Offline source/package and production C++ rules only; not Unreal gameplay validation",
              "engine_preflight": engine,
              "unreal_tests": {"status": "Blockiert" if engine["issues"] else "Nicht ausgeführt", "registered_tests": sorted(tests)},
              "windows_build": {"status": "Blockiert" if engine["issues"] else "Nicht ausgeführt"},
              "levels": {"actual_umap_count": len(list((ROOT / "Content").rglob("*.umap"))), "acceptance": "Nicht ausgeführt"},
              "performance": {"status": "Blockiert", "fps": None, "reason": "No running Unreal game available for measurement"}}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")
    for name, result in checks.items():
        print(name + ": " + result["status"])
    print("Unreal/Win64: " + report["windows_build"]["status"])
    print("Report: " + str(args.report))
    return 1 if any(value["status"] == "Fehlgeschlagen" for value in checks.values()) else 0


if __name__ == "__main__":
    raise SystemExit(main())
