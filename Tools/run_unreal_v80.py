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
import math
import importlib.util
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import time

MAP_PATH = "/Game/Askir/Maps/Dornfeld_V80"
OUTSKIRTS_PATH = "/Game/Askir/Maps/Dornfelder_Umland_V80"
MAP_PATHS = (MAP_PATH, OUTSKIRTS_PATH)
REPORT_STATUSES = ("Bestanden", "Fehlgeschlagen", "Nicht ausgeführt", "Blockiert")
PHASES = ("environment", "backup", "editor_build", "map_generation", "automation", "windows_package", "packaged_startup")


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def readiness_map(log_text, expected="Dornfeld_V80"):
    markers = [line for line in log_text.splitlines() if "ASKIR_V80_RUNTIME_READY" in line]
    if not markers:
        return None
    match = re.search(r"\bmap=([^\s]+)", markers[-1])
    value = match.group(1) if match else "<missing map>"
    if value != expected:
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
    report_path = Path(report_path);project_root = Path(project_root)
    if not report_path.is_file(): raise RuntimeError("World script supplied no report")
    data = json.loads(report_path.read_text(encoding="utf-8"))
    if data.get("status") != "Bestanden" or data.get("map_package") != MAP_PATH:
        raise RuntimeError("World generation/validation failed: " + str(data.get("error", data)))
    if data.get("gameplay_status") != "Nicht ausgeführt":
        raise RuntimeError("Editor report must not claim an unexecuted runtime playthrough")
    spec = importlib.util.spec_from_file_location("_v80_map_report_contract", Path(__file__).with_name("build_dornfeld_v80.py"))
    builder = importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
    contract = builder.source_preflight(project_root)
    maps = data.get("maps", {})
    if set(maps) != set(MAP_PATHS): raise RuntimeError("Both V80 world validation records are required")
    routes = data.get("reciprocal_routes", {})
    if routes.get("status") != "Bestanden" or routes.get("portal_ids") != [entry["portal"]["id"] for entry in contract["maps"]]:
        raise RuntimeError("Reciprocal route evidence missing")
    expected_packages = {contract["reward_asset"]} | set(builder.PROP_PACKAGES.values())
    for role in ("Hero", "Guard", "Raider"):
        expected_packages.add("/Game/Askir/CharactersV78/"+role+"/SK_Askir"+role)
        expected_packages.update("/Game/Askir/CharactersV78/"+role+"/A_"+clip for clip in ("Idle","Walk","Run","Sprint","Jump","Dodge","LightAttack","HeavyAttack","Hit","Death","Block","Cast"))
    files = []
    for entry in contract["maps"]:
        package = entry["package"];result = maps[package]
        physical = project_root / ("Content/" + package.removeprefix("/Game/") + ".umap")
        if not physical.is_file() or physical.stat().st_size < 1024: raise RuntimeError("Real saved world package missing: " + str(physical))
        if result.get("status") != "Bestanden" or result.get("map_reload") is not True or result.get("world_asset_type") != "UWorld":
            raise RuntimeError("Real UWorld reload evidence missing: " + package)
        for key in ("native_game_mode", "objective_references", "spawn_collision_traces", "arrival_collision", "delivery_props"):
            if result.get(key) != "Bestanden": raise RuntimeError("World validation missing: " + key)
        extent = result.get("player_capsule_extent",[])
        if len(extent) != 3 or any(isinstance(value,bool) or not isinstance(value,(int,float)) or not math.isfinite(value) or value <= 0 for value in extent) or extent[0] != extent[1] or extent[2] < extent[0]:
            raise RuntimeError("Actual native player capsule dimensions missing/invalid")
        if all(abs(entry["portal"]["arrival_offset"][axis]) <= entry["portal"]["trigger_extent"][axis]+extent[axis] for axis in range(3)):
            raise RuntimeError("Actual native arrival capsule intersects portal trigger")
        nav = result.get("navigation_paths", {})
        if nav.get("status") != "Bestanden" or nav.get("targets") != builder.expected_targets(contract, package):
            raise RuntimeError("Objective navigation evidence incomplete: " + package)
        if result.get("route") != entry["portal"]: raise RuntimeError("Saved route identity or arrival geometry differs")
        packages = result.get("asset_package_files", [])
        if not expected_packages.issubset(packages): raise RuntimeError("Required reward/rig package evidence incomplete")
        skeletons = result.get("animation_skeletons",[])
        if result.get("animation_sequences") != "Bestanden" or len(set(skeletons)) != 3 or not set(skeletons).issubset(packages): raise RuntimeError("Required exact-skeleton/animation-duration validation missing")
        if any(not any(path.startswith("/Game/Askir/CharactersV78/"+role+"/") for path in skeletons) for role in ("Hero","Guard","Raider")):
            raise RuntimeError("Required skeleton package for each role missing")
        for asset in packages:
            if not isinstance(asset,str) or not asset.startswith("/Game/") or any(part in (".","..","") for part in asset[6:].split("/")) or "\\" in asset:
                raise RuntimeError("Unsafe package path in world report")
            physical_asset = project_root / ("Content/" + asset[6:] + ".uasset")
            if not physical_asset.is_file() or physical_asset.stat().st_size < 128: raise RuntimeError("Real asset package missing: " + asset)
        files.append(str(physical))
    inherited = maps[MAP_PATH]
    for key in ("relay_source_references", "navigation_brush_geometry", "audio_asset_references"):
        if inherited.get(key) != "Bestanden": raise RuntimeError("Inherited well mission validation missing: "+key)
    village = builder.module("_v80_runner_economy", project_root / "Tools/build_dornfeld_v79.py")
    economy_contract = village.source_preflight(project_root)
    economy = inherited.get("economy", {})
    expected_economy = {village.MERCHANT_LABEL, village.STATION_LABEL} | {entry["id"] for entry in economy_contract["resource_pickups"]}
    if economy.get("status") != "Bestanden" or economy.get("merchant_id") != economy_contract["merchant_id"] or set(economy.get("navigation_targets", [])) != expected_economy or economy.get("recipe_count") != len(economy_contract["recipes"]):
        raise RuntimeError("Preserved V79 merchant/crafting validation incomplete")
    expected_economy_packages = {economy_contract["asset_root"]+"/"+entry["id"] for entry in economy_contract["items"]}
    expected_audio = {"/Game/Askir/V77/Audio/S_"+cue for cue in ("Shard","Chest","QuestAccepted","QuestCompleted","MeleeSwing","MeleeHit","DungeonAmbient")} | {"/Game/Askir/V78/Audio/S_SpellCast","/Game/Askir/V78/Audio/S_SpellImpact"}
    original = builder.module("_v80_runner_well",project_root / "Tools/build_dornfeld_v77.py")
    expected_well = {"/Game/Askir/V77/Items/"+name for name in original.design_contract()["shard_ids"]+["Dornfeld_WellRelic"]} | {"/Game/Askir/V77/Materials/"+name for name in original.MATERIAL_NAMES}
    if not (expected_economy_packages | expected_audio | expected_well).issubset(inherited.get("asset_package_files",[])):
        raise RuntimeError("Inherited well/economy/audio package evidence incomplete")
    startup = data.get("startup_map_config", {})
    if startup.get("status") != "Bestanden" or startup.get("game_default_map") != MAP_PATH or startup.get("editor_startup_map") != MAP_PATH or startup.get("maps_to_cook") != list(MAP_PATHS):
        raise RuntimeError("Validated two-map startup/cook configuration missing")
    try: config = (project_root / "Config/DefaultEngine.ini").read_text(encoding="utf-8-sig")
    except OSError as exc: raise RuntimeError("Actual startup config missing") from exc
    for key in ("GameDefaultMap", "EditorStartupMap"):
        if re.findall(r"^\s*"+key+r"\s*=\s*(\S+)\s*$",config,re.MULTILINE) != [MAP_PATH]: raise RuntimeError("Actual startup config differs: "+key)
    return {"status":"Bestanden","report":str(report_path),"map_files":files,"coverage":data.get("coverage"),"gameplay_status":data.get("gameplay_status")}


class Pipeline:
    def __init__(self, project, engine_root, output, timeout=3600, use_existing_map=False):
        self.project = Path(project).resolve()
        self.root = self.project.parent
        self.engine = engine_root
        self.output = Path(output).resolve()
        self.output.mkdir(parents=True, exist_ok=True)
        self.timeout = timeout
        self.use_existing_map = use_existing_map
        self.report = {"version": "V80", "started_at": utc_now(), "project": str(self.project),
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
        destination = self.output / "Backup/Askir_pre_v80_generation"
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
        script = self.root / "Tools/build_dornfeld_v80.py"
        map_report = self.output / "world_editor.json"
        if map_report.exists():
            raise RuntimeError("Map-Bericht existiert schon; neuer Ausgabepfad verhindert falsche Altnachweise")
        env = dict(os.environ, ASKIR_V80_PROJECT=str(self.root), ASKIR_V80_MAP_REPORT=str(map_report))
        if self.use_existing_map:
            env["ASKIR_V80_VALIDATE_ONLY"] = "1"
        tool = self.engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
        command_result = self.command("map_generation", [tool, self.project, "-run=pythonscript", "-script=" + str(script),
                                                         "-unattended", "-nop4", "-nosplash", "-stdout", "-FullStdOutLogOutput"], env=env)
        command_result.update(check_map_report(map_report, self.root))
        return command_result

    def automation(self):
        results = {}
        for package in MAP_PATHS:
            short = package.rsplit("/",1)[-1];target = self.output / "Automation" / short
            if target.exists(): raise RuntimeError("Refusing stale automation reports: "+str(target))
            tool = self.engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
            result = self.command("automation_"+short,[tool,self.project,package,"-unattended","-nop4","-nosplash","-NullRHI","-stdout",
                "-FullStdOutLogOutput","-ExecCmds=Automation RunTests Askir.","-TestExit=Automation Test Queue Empty","-ReportExportPath="+str(target)])
            result.update(automation_result(target,required_test_names(self.root)));results[package] = result
        return {"status":"Bestanden","maps":results,"coverage":"registered native tests executed in each map; travel/playthrough not tested"}

    def windows_package(self):
        target = self.output / "WindowsBuild"
        if target.exists():
            raise RuntimeError("WindowsBuild-Ziel existiert bereits")
        tool = self.engine / "Engine/Build/BatchFiles/RunUAT.bat"
        result = self.command("windows_package", self.batch(tool, ["BuildCookRun", "-project=" + str(self.project),
                              "-noP4", "-platform=Win64", "-clientconfig=Development", "-target=Askir",
                              "-build", "-cook", "-map=" + "+".join(MAP_PATHS), "-stage", "-pak", "-archive",
                              "-archivedirectory=" + str(target), "-unattended", "-utf8output"]))
        executables = list(target.glob("**/Binaries/Win64/Askir.exe"))
        if len(executables) != 1 or executables[0].stat().st_size < 1024:
            raise RuntimeError("UAT meldet Erfolg, aber genau eine echte Askir.exe fehlt")
        # Use the executable inside Binaries rather than the bootstrap launcher.
        result["executable"] = str(executables[0])
        result["coverage"] = "development Win64 build/cook/stage/archive of both V80 maps"
        return result

    def _packaged_map_startup(self, expected, override=None):
        executable = Path(self.report["phases"]["windows_package"]["executable"])
        log = self.output / ("packaged_startup_"+expected+".log")
        console_log = self.output / ("packaged_startup_"+expected+"_console.log")
        # No explicit map override: verify the package starts through GameDefaultMap as a player would.
        args = [str(executable)] + ([override] if override else []) + ["-unattended", "-nosplash", "-NullRHI", "-nosound", "-stdout",
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
                    if "ASKIR_V80_RUNTIME_BLOCKED" in log_text:
                        raise RuntimeError("Paketierter Build meldet blockierte Spielerinitialisierung; " + str(log))
                    if readiness_map(log_text,expected) == expected:
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
                "coverage": "headless packaged map startup; visual/input/travel/end-to-end gameplay not tested"}

    def packaged_startup(self):
        results = {MAP_PATH:self._packaged_map_startup("Dornfeld_V80"),
                   OUTSKIRTS_PATH:self._packaged_map_startup("Dornfelder_Umland_V80",OUTSKIRTS_PATH)}
        return {"status":"Bestanden","maps":results,"coverage":"default village startup plus explicit countryside startup; reciprocal runtime travel not executed"}

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
    parser.add_argument("--use-existing-map", action="store_true", help="Load/validate both existing V80 maps; preserve their content")
    args = parser.parse_args(argv)
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    pipeline = Pipeline(args.project, discover_engine(args.engine_root), args.output, args.timeout, args.use_existing_map)
    result = pipeline.run(args.check_only)
    print("ASKIR V80:", pipeline.report["overall_status"], "—", pipeline.output / "pipeline.json")
    return result


if __name__ == "__main__":
    sys.exit(main())
