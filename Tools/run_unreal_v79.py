#!/usr/bin/env python3
"""Fail-closed UE5.6 Win64 build, real-map generation, tests, cook and startup runner.

Uses argv arrays (no shell interpolation). Engine return codes alone are insufficient:
map creation needs its success report, automation needs nonempty passing results,
packaging needs a real executable, startup needs the game's readiness log marker.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import importlib.util
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import time

MAP_PATH = "/Game/Askir/Maps/Dornfeld_V79"
REPORT_STATUSES = ("Bestanden", "Fehlgeschlagen", "Nicht ausgeführt", "Blockiert")
PHASES = ("environment", "backup", "editor_build", "map_generation", "automation", "windows_package", "packaged_startup")


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def readiness_map(log_text):
    markers = [line for line in log_text.splitlines() if "ASKIR_V79_RUNTIME_READY" in line]
    if not markers:
        return None
    match = re.search(r"\bmap=([^\s]+)", markers[-1])
    value = match.group(1) if match else "<missing map>"
    if value != "Dornfeld_V79":
        raise RuntimeError("Paketierter Build startet falsche Default-Map: " + markers[-1])
    return value


def write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_suffix(path.suffix + ".tmp")
    temp.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding="utf-8")
    temp.replace(path)


def discover_engine(explicit=None):
    if explicit is not None:
        return Path(explicit).resolve()
    if os.environ.get("UE56_ROOT"):
        return Path(os.environ["UE56_ROOT"]).resolve()
    candidates = ["C:/Program Files/Epic Games/UE_5.6", "/opt/UnrealEngine-5.6", "/opt/UnrealEngine"]
    return next((Path(value).resolve() for value in candidates if value and (Path(value) / "Engine/Build/Build.version").is_file()), None)


def preflight(project, engine_root, target_platform=None):
    target_platform = target_platform or platform.system()
    issues = []
    project_data = {}
    try:
        project_data = json.loads(Path(project).read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        issues.append("Projektdatei fehlt oder ungültig: " + str(exc))
    if project_data and project_data.get("EngineAssociation") != "5.6":
        issues.append(".uproject EngineAssociation muss 5.6 sein")
    plugins = {entry["Name"]: entry.get("Enabled", False) for entry in project_data.get("Plugins", [])}
    for name in ("PythonScriptPlugin", "EditorScriptingUtilities"):
        if not plugins.get(name):
            issues.append("Editor-Plugin muss im Projekt aktiv sein: " + name)
    version = None
    if engine_root is None:
        issues.append("Unreal Engine 5.6 nicht gefunden; --engine-root oder UE56_ROOT setzen")
    else:
        try:
            version = json.loads((engine_root / "Engine/Build/Build.version").read_text(encoding="utf-8"))
            if (version.get("MajorVersion"), version.get("MinorVersion")) != (5, 6):
                issues.append("Engine ist nicht Unreal 5.6")
        except (OSError, ValueError) as exc:
            issues.append("Engine Build.version fehlt oder ungültig: " + str(exc))
        for relative in ("Engine/Build/BatchFiles/Build.bat", "Engine/Build/BatchFiles/RunUAT.bat",
                         "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"):
            if not (engine_root / relative).is_file():
                issues.append("Benötigtes Windows-Engine-Werkzeug fehlt: " + relative)
    if target_platform != "Windows":
        issues.append("Win64 UBT/Cook/EXE-Prüfung benötigt einen Windows-Host; aktuell " + target_platform)
    return {"status": "Blockiert" if issues else "Bestanden", "checked_at": utc_now(),
            "host_platform": target_platform, "engine_root": str(engine_root) if engine_root else None,
            "engine_version": version, "issues": issues,
            "note": "Compiler und SDK werden zusätzlich durch UnrealBuildTool geprüft"}


def required_test_names(project_root):
    result = set()
    for path in (Path(project_root) / "Source/Askir/Tests").glob("*.cpp"):
        result.update(re.findall(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\s*\([^,]+,\s*"([^"]+)"', path.read_text(encoding="utf-8")))
    if not result:
        raise RuntimeError("Keine registrierten Askir-Automation-Tests im Quellcode gefunden")
    return result


def automation_result(report_dir, expected):
    path = Path(report_dir) / "index.json"
    if not path.is_file():
        raise RuntimeError("Automation lieferte keinen index.json-Bericht")
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    leaves = []

    def walk(nodes):
        for entry in nodes:
            if entry.get("children"):
                walk(entry["children"])
            else:
                leaves.append(entry)

    walk(data.get("tests", []))
    if not leaves:
        raise RuntimeError("Automation meldete keine ausgeführten Tests")
    names = {entry.get("testFullPath", entry.get("fullTestPath", "")): entry for entry in leaves}
    missing = sorted(expected - names.keys())
    if missing:
        raise RuntimeError("Automation führte erforderliche Tests nicht aus: " + ", ".join(missing))
    bad = [name or entry.get("testDisplayName", "unknown") for name, entry in names.items() if entry.get("state") != "Success"]
    counters = {key: data.get(key, 0) for key in ("failed", "notRun", "inProcess")}
    if bad or any(value for value in counters.values()):
        raise RuntimeError("Automation nicht vollständig bestanden: " + json.dumps({"tests": bad, **counters}))
    return {"status": "Bestanden", "test_count": len(leaves), "expected_test_count": len(expected),
            "report": str(path), "coverage": "registered native automation tests; does not imply manual gameplay QA"}


def check_map_report(report_path, project_root):
    report_path = Path(report_path)
    if not report_path.is_file():
        raise RuntimeError("Map-Script lieferte keinen Bericht; Exit-Code allein ist kein Erfolgsnachweis")
    data = json.loads(report_path.read_text(encoding="utf-8"))
    path = Path(project_root) / "Content/Askir/Maps/Dornfeld_V79.umap"
    if data.get("status") != "Bestanden" or data.get("map_package") != MAP_PATH:
        raise RuntimeError("Map-Generierung/Validierung fehlgeschlagen: " + str(data.get("error", data)))
    if not path.is_file() or path.stat().st_size < 1024:
        raise RuntimeError("Echte gespeicherte .umap fehlt trotz Erfolgsbericht")
    if not data.get("validation", {}).get("map_reload"):
        raise RuntimeError("Gespeicherte Map wurde nicht erfolgreich wieder geladen")
    startup = data.get("startup_map_config", {})
    if startup.get("status") != "Bestanden" or startup.get("game_default_map") != MAP_PATH or startup.get("editor_startup_map") != MAP_PATH:
        raise RuntimeError("Map-Bericht enthält keine validierte Default-/Editor-Startup-Konfiguration")
    config_path = Path(project_root) / "Config/DefaultEngine.ini"
    try:
        config_text = config_path.read_text(encoding="utf-8-sig")
    except OSError as exc:
        raise RuntimeError("DefaultEngine.ini fehlt nach Map-Generierung") from exc
    for key in ("GameDefaultMap", "EditorStartupMap"):
        values = re.findall(r"^\s*" + key + r"\s*=\s*(\S+)\s*$", config_text, re.MULTILINE)
        if values != [MAP_PATH]:
            raise RuntimeError("Tatsächliche DefaultEngine.ini enthält falsche/mehrfache " + key)
    economy = data.get("validation", {}).get("economy", {})
    spec = importlib.util.spec_from_file_location("_askir_v79_map_report_contract", Path(__file__).with_name("build_dornfeld_v79.py"))
    village = importlib.util.module_from_spec(spec);spec.loader.exec_module(village)
    contract = village.source_preflight(Path(project_root))
    expected_targets = {village.MERCHANT_LABEL, village.STATION_LABEL} | {entry["id"] for entry in contract["resource_pickups"]}
    if economy.get("status") != "Bestanden" or economy.get("merchant_id") != contract["merchant_id"]:
        raise RuntimeError("V79 gespeicherte Händler-/Crafting-Validierung fehlt")
    if set(economy.get("navigation_targets", [])) != expected_targets or economy.get("recipe_count") != len(contract["recipes"]):
        raise RuntimeError("V79 Economy Navigations-/Rezeptnachweis unvollständig")
    packages = data.get("validation", {}).get("asset_package_files", [])
    expected_items = {contract["asset_root"] + "/" + item["id"] for item in contract["items"]}
    if not expected_items.issubset(packages):
        raise RuntimeError("V79 Map-Bericht enthält keine vollständigen Itempakete")
    if not packages:
        raise RuntimeError("Map-Bericht enthält keine gespeicherten Asset-Dateinachweise")
    for package in packages:
        if not package.startswith("/Game/") or ".." in package:
            raise RuntimeError("Ungültiger Projektpaketpfad im Map-Bericht")
        asset_file = Path(project_root) / "Content" / (package[len("/Game/"):] + ".uasset")
        if not asset_file.is_file() or asset_file.stat().st_size < 128:
            raise RuntimeError("Reale benötigte .uasset fehlt trotz Erfolgsbericht: " + str(asset_file))
    return {"status": "Bestanden", "report": str(report_path), "map_file": str(path),
            "coverage": data.get("coverage"), "gameplay_status": data.get("gameplay_status")}


class Pipeline:
    def __init__(self, project, engine_root, output, timeout=3600, use_existing_map=False):
        self.project = Path(project).resolve()
        self.root = self.project.parent
        self.engine = engine_root
        self.output = Path(output).resolve()
        self.output.mkdir(parents=True, exist_ok=True)
        self.timeout = timeout
        self.use_existing_map = use_existing_map
        self.report = {"version": "V79", "started_at": utc_now(), "project": str(self.project),
                       "map": MAP_PATH, "overall_status": "Nicht ausgeführt",
                       "phases": {phase: {"status": "Nicht ausgeführt"} for phase in PHASES},
                       "game_acceptance": "Nicht ausgeführt"}

    def save(self):
        write_json(self.output / "pipeline.json", self.report)

    def command(self, phase, args, env=None):
        log = self.output / (phase + ".log")
        started = time.monotonic()
        with log.open("w", encoding="utf-8") as handle:
            command = args if isinstance(args, str) else [str(arg) for arg in args]
            handle.write("Command: " + json.dumps(command, ensure_ascii=False) + "\n")
            handle.flush()
            try:
                process = subprocess.Popen(command, cwd=self.root, env=env,
                                           stdout=handle, stderr=subprocess.STDOUT,
                                           creationflags=getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0))
                process.wait(timeout=self.timeout)
            except subprocess.TimeoutExpired as exc:
                if platform.system() == "Windows":
                    subprocess.run(["taskkill.exe", "/PID", str(process.pid), "/T", "/F"],
                                   stdout=handle, stderr=subprocess.STDOUT, timeout=30, check=False)
                else:
                    process.kill()
                process.wait(timeout=30)
                raise RuntimeError("Zeitlimit überschritten: " + phase + "; Log " + str(log)) from exc
        if process.returncode:
            raise RuntimeError(phase + " Exit-Code " + str(process.returncode) + "; Log " + str(log))
        return {"status": "Bestanden", "log": str(log), "exit_code": process.returncode,
                "duration_seconds": round(time.monotonic() - started, 3)}

    def batch(self, path, args):
        # list2cmdline quotes every argument according to Windows rules; paths are never shell snippets.
        # Reject cmd metacharacters even inside quotes: cmd.exe's percent expansion is not shell-safe.
        values = [str(path)] + [str(arg) for arg in args]
        cmd = os.environ.get("COMSPEC", "cmd.exe")
        if any(any(char in value for char in ('%', '!', '\n', '\r', '&', '|', '<', '>', '^', '"', '(', ')')) for value in values + [cmd]):
            raise RuntimeError("Für cmd.exe nicht sicher darstellbarer Pfad/Parameter")
        # Pass a raw command line to CreateProcess on Windows. A list here would quote/escape
        # the already-quoted /c payload a second time; cmd.exe does not use CRT backslash rules.
        return subprocess.list2cmdline([cmd]) + ' /d /s /c "' + subprocess.list2cmdline(values) + '"'

    def backup(self):
        # Keep all source/config/assets. Generated caches/build products are excluded explicitly.
        destination = self.output / "Backup/Askir_pre_v79_generation"
        ignored = shutil.ignore_patterns(".git", "Binaries", "Intermediate", "DerivedDataCache", "Saved")
        if destination.exists():
            raise RuntimeError("Sicherungsziel existiert bereits: " + str(destination))
        if self.output.is_relative_to(self.root):
            raise RuntimeError("Pipeline-Ausgabe/Sicherung muss außerhalb des Projektordners liegen")
        shutil.copytree(self.root, destination, ignore=ignored)
        return {"status": "Bestanden", "path": str(destination), "coverage": "source/config/content backup before editor mutation"}

    def editor_build(self):
        tool = self.engine / "Engine/Build/BatchFiles/Build.bat"
        return self.command("editor_build", self.batch(tool, ["AskirEditor", "Win64", "Development", "-Project=" + str(self.project), "-WaitMutex", "-NoHotReloadFromIDE"]))

    def map_generation(self):
        script = self.root / "Tools/build_dornfeld_v79.py"
        map_report = self.output / "dornfeld_editor.json"
        if map_report.exists():
            raise RuntimeError("Map-Bericht existiert schon; neuer Ausgabepfad verhindert falsche Altnachweise")
        env = dict(os.environ, ASKIR_V79_PROJECT=str(self.root), ASKIR_V79_MAP_REPORT=str(map_report))
        if self.use_existing_map:
            env["ASKIR_V79_VALIDATE_ONLY"] = "1"
        tool = self.engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
        command_result = self.command("map_generation", [tool, self.project, "-run=pythonscript", "-script=" + str(script),
                                                         "-unattended", "-nop4", "-nosplash", "-stdout", "-FullStdOutLogOutput"], env=env)
        command_result.update(check_map_report(map_report, self.root))
        return command_result

    def automation(self):
        target = self.output / "Automation"
        if target.exists():
            raise RuntimeError("Automation-Ziel existiert; Wiederverwendung alter Testberichte verboten")
        tool = self.engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
        result = self.command("automation", [tool, self.project, MAP_PATH, "-unattended", "-nop4", "-nosplash", "-NullRHI", "-stdout",
                                              "-FullStdOutLogOutput", "-ExecCmds=Automation RunTests Askir.",
                                              "-TestExit=Automation Test Queue Empty", "-ReportExportPath=" + str(target)])
        result.update(automation_result(target, required_test_names(self.root)))
        return result

    def windows_package(self):
        target = self.output / "WindowsBuild"
        if target.exists():
            raise RuntimeError("WindowsBuild-Ziel existiert bereits")
        tool = self.engine / "Engine/Build/BatchFiles/RunUAT.bat"
        result = self.command("windows_package", self.batch(tool, ["BuildCookRun", "-project=" + str(self.project),
                              "-noP4", "-platform=Win64", "-clientconfig=Development", "-target=Askir",
                              "-build", "-cook", "-map=" + MAP_PATH, "-stage", "-pak", "-archive",
                              "-archivedirectory=" + str(target), "-unattended", "-utf8output"]))
        executables = list(target.glob("**/Binaries/Win64/Askir.exe"))
        if len(executables) != 1 or executables[0].stat().st_size < 1024:
            raise RuntimeError("UAT meldet Erfolg, aber genau eine echte Askir.exe fehlt")
        # Use the executable inside Binaries rather than the bootstrap launcher.
        result["executable"] = str(executables[0])
        result["coverage"] = "development Win64 build/cook/stage/archive of Dornfeld only"
        return result

    def packaged_startup(self):
        executable = Path(self.report["phases"]["windows_package"]["executable"])
        log = self.output / "packaged_startup.log"
        console_log = self.output / "packaged_startup_console.log"
        # No explicit map override: verify the package starts through GameDefaultMap as a player would.
        args = [str(executable), "-unattended", "-nosplash", "-NullRHI", "-nosound", "-stdout",
                "-FullStdOutLogOutput", "-abslog=" + str(log)]
        started = time.monotonic()
        with console_log.open("w", encoding="utf-8") as handle:
            process = subprocess.Popen(args, cwd=executable.parent, stdout=handle, stderr=subprocess.STDOUT)
            ready = False
            log_text = ""
            try:
                deadline = started + min(self.timeout, 90)
                while time.monotonic() < deadline:
                    log_text = log.read_text(encoding="utf-8", errors="replace") if log.is_file() else ""
                    if "Fatal error:" in log_text or "Assertion failed:" in log_text:
                        raise RuntimeError("Paketierter Build meldet fatalen Fehler; " + str(log))
                    if "ASKIR_V79_RUNTIME_BLOCKED" in log_text:
                        raise RuntimeError("Paketierter Build meldet blockierte Spielerinitialisierung; " + str(log))
                    if readiness_map(log_text) == "Dornfeld_V79":
                        ready = True
                        break
                    if process.poll() is not None:
                        raise RuntimeError("Paketierter Build endete vor Runtime-Bereitschaft, Exit " + str(process.returncode))
                    time.sleep(.25)
                if not ready:
                    raise RuntimeError("Kein Runtime-Bereitschaftsnachweis innerhalb 90s; " + str(log))
                # Observe beyond initialization to catch immediate post-start crashes.
                observation_deadline = time.monotonic() + 5
                while time.monotonic() < observation_deadline:
                    observed_log = log.read_text(encoding="utf-8", errors="replace") if log.is_file() else ""
                    if "Fatal error:" in observed_log or "Assertion failed:" in observed_log:
                        raise RuntimeError("Paketierter Build meldet Fehler nach Start; " + str(log))
                    if process.poll() is not None:
                        raise RuntimeError("Paketierter Build endete unmittelbar nach Start, Exit " + str(process.returncode))
                    time.sleep(.25)
            finally:
                if process.poll() is None:
                    process.terminate()
                    try:
                        process.wait(timeout=10)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait(timeout=10)
        return {"status": "Bestanden", "log": str(log), "console_log": str(console_log),
                "duration_seconds": round(time.monotonic() - started, 3), "readiness_marker": ready,
                "termination": "stopped by runner after five-second startup observation",
                "coverage": "headless packaged default-map startup in Dornfeld_V79; visual/input/end-to-end gameplay not tested"}

    def run(self, check_only=False):
        self.report["phases"]["environment"] = preflight(self.project, self.engine)
        self.save()
        if self.report["phases"]["environment"]["status"] == "Blockiert":
            self.report["overall_status"] = "Blockiert"
            self.save()
            return 2
        if check_only:
            self.report["overall_status"] = "Nicht ausgeführt"
            self.save()
            return 0
        for phase in PHASES[1:]:
            try:
                self.report["phases"][phase] = getattr(self, phase)()
                self.save()
            except Exception as exc:
                self.report["phases"][phase] = {"status": "Fehlgeschlagen", "error": str(exc)}
                self.report["overall_status"] = "Fehlgeschlagen"
                self.save()
                return 1
        self.report["finished_at"] = utc_now()
        self.report["overall_status"] = "Bestanden"
        self.report["game_acceptance"] = "Nicht ausgeführt"
        self.report["remaining"] = ["manual gameplay QA, full 30-level campaign, production assets and performance measurement"]
        self.save()
        return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, default=Path(__file__).resolve().parents[1] / "AskirRPG.uproject")
    parser.add_argument("--engine-root", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="New external directory for backup/build/logs")
    parser.add_argument("--timeout", type=int, default=3600, help="Max seconds per engine command")
    parser.add_argument("--check-only", action="store_true")
    parser.add_argument("--use-existing-map", action="store_true", help="Load/validate existing V79 map; preserve its content")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    pipeline = Pipeline(args.project, discover_engine(args.engine_root), args.output, args.timeout, args.use_existing_map)
    result = pipeline.run(args.check_only)
    print("ASKIR V79:", pipeline.report["overall_status"], "—", pipeline.output / "pipeline.json")
    return result


if __name__ == "__main__":
    sys.exit(main())
