import json
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
DATA = json.loads((ROOT / "Production/Blueprints/ai_profiles_v53.json").read_text())
PROFILES = DATA["profiles"]
MANIFEST = json.loads((ROOT / "Production/Blueprints/blueprint_manifest_v50.json").read_text())
ENEMIES = {e["name"] for e in MANIFEST["blueprints"] if e["category"] == "Enemies"}
CPP = (ROOT / "Source/Askir/AI/AskirEnemyAIController.cpp").read_text()
SCRIPT = (ROOT / "Tools/configure_individual_ai_v53.py").read_text()

class AIProfilesV53(unittest.TestCase):
    pass

def check(idx, label, predicate):
    def test(self):
        self.assertTrue(predicate(), label)
    setattr(AIProfilesV53, "test_%02d_%s" % (idx, label), test)

check(1, "engine", lambda: DATA["engine"] == "5.6")
check(2, "version", lambda: DATA["version"] == 53)
check(3, "six_profiles", lambda: len(PROFILES) == 6)
check(4, "all_enemies", lambda: {p["pawn"] for p in PROFILES} == ENEMIES)
check(5, "unique_controllers", lambda: len({p["controller_path"] for p in PROFILES}) == 6)
check(6, "unique_pawns", lambda: len({p["pawn_path"] for p in PROFILES}) == 6)
check(7, "controller_paths", lambda: all(p["controller_path"].startswith("/Game/Askir/Blueprints/AI/") for p in PROFILES))
check(8, "native_controller", lambda: 'BASE_CONTROLLER = "/Script/Askir.AskirEnemyAIController"' in SCRIPT)
check(9, "dry_run_default", lambda: "DRY_RUN = True" in SCRIPT)
check(10, "no_existing_overwrite", lambda: "APPLY_EXISTING_PRESETS = False" in SCRIPT)
check(11, "safe_missing", lambda: 'report["missing"]' in SCRIPT)
check(12, "safe_errors", lambda: 'report["errors"]' in SCRIPT)
check(13, "save_assets", lambda: 'lib.save_loaded_asset(bp)' in SCRIPT and 'lib.save_loaded_asset(pawn_bp)' in SCRIPT)
check(14, "auto_possess", lambda: "PLACED_IN_WORLD_OR_SPAWNED" in SCRIPT)
check(15, "ai_class", lambda: '"ai_controller_class"' in SCRIPT)
check(16, "memory_clamped", lambda: "FMath::Max(0.f, MemorySeconds)" in CPP)
check(17, "patrol_reset", lambda: "if (State == EAskirEnemyState::Patrol) StopMovement();" in CPP)
check(18, "memory_position", lambda: "if (bVisible) LastKnownTargetLocation = Target->GetActorLocation();" in CPP)
check(19, "clears_focus", lambda: "ClearFocus(EAIFocusPriority::Gameplay)" in CPP)
check(20, "no_native_mutation", lambda: 'unreal.load_class(None, BASE_CONTROLLER)' in SCRIPT)
check(21, "bp_bandit_sight", lambda: 500 <= PROFILES[0]['sight_range'] <= 5000)
check(22, "bp_bandit_attack_distance", lambda: 50 <= PROFILES[0]['attack_distance'] <= 500)
check(23, "bp_bandit_attack_interval", lambda: 0.2 <= PROFILES[0]['attack_interval'] <= 10)
check(24, "bp_bandit_hearing", lambda: 100 <= PROFILES[0]['hearing_range'] <= 5000)
check(25, "bp_bandit_retreat", lambda: 0 <= PROFILES[0]['retreat_health_fraction'] < 1)
check(26, "bp_wolf_sight", lambda: 500 <= PROFILES[1]['sight_range'] <= 5000)
check(27, "bp_wolf_attack_distance", lambda: 50 <= PROFILES[1]['attack_distance'] <= 500)
check(28, "bp_wolf_attack_interval", lambda: 0.2 <= PROFILES[1]['attack_interval'] <= 10)
check(29, "bp_wolf_hearing", lambda: 100 <= PROFILES[1]['hearing_range'] <= 5000)
check(30, "bp_wolf_retreat", lambda: 0 <= PROFILES[1]['retreat_health_fraction'] < 1)
check(31, "bp_skeleton_sight", lambda: 500 <= PROFILES[2]['sight_range'] <= 5000)
check(32, "bp_skeleton_attack_distance", lambda: 50 <= PROFILES[2]['attack_distance'] <= 500)
check(33, "bp_skeleton_attack_interval", lambda: 0.2 <= PROFILES[2]['attack_interval'] <= 10)
check(34, "bp_skeleton_hearing", lambda: 100 <= PROFILES[2]['hearing_range'] <= 5000)
check(35, "bp_skeleton_retreat", lambda: 0 <= PROFILES[2]['retreat_health_fraction'] < 1)
check(36, "bp_cultist_sight", lambda: 500 <= PROFILES[3]['sight_range'] <= 5000)
check(37, "bp_cultist_attack_distance", lambda: 50 <= PROFILES[3]['attack_distance'] <= 500)
check(38, "bp_cultist_attack_interval", lambda: 0.2 <= PROFILES[3]['attack_interval'] <= 10)
check(39, "bp_cultist_hearing", lambda: 100 <= PROFILES[3]['hearing_range'] <= 5000)
check(40, "bp_cultist_retreat", lambda: 0 <= PROFILES[3]['retreat_health_fraction'] < 1)
check(41, "bp_eliteguard_sight", lambda: 500 <= PROFILES[4]['sight_range'] <= 5000)
check(42, "bp_eliteguard_attack_distance", lambda: 50 <= PROFILES[4]['attack_distance'] <= 500)
check(43, "bp_eliteguard_attack_interval", lambda: 0.2 <= PROFILES[4]['attack_interval'] <= 10)
check(44, "bp_eliteguard_hearing", lambda: 100 <= PROFILES[4]['hearing_range'] <= 5000)
check(45, "bp_eliteguard_retreat", lambda: 0 <= PROFILES[4]['retreat_health_fraction'] < 1)
check(46, "bp_askirboss_sight", lambda: 500 <= PROFILES[5]['sight_range'] <= 5000)
check(47, "bp_askirboss_attack_distance", lambda: 50 <= PROFILES[5]['attack_distance'] <= 500)
check(48, "bp_askirboss_attack_interval", lambda: 0.2 <= PROFILES[5]['attack_interval'] <= 10)
check(49, "bp_askirboss_hearing", lambda: 100 <= PROFILES[5]['hearing_range'] <= 5000)
check(50, "bp_askirboss_retreat", lambda: 0 <= PROFILES[5]['retreat_health_fraction'] < 1)

if __name__ == "__main__": unittest.main()
