"""Generate the runtime C++ catalog from the canonical 24 spell records."""
import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCHOOLS = {"Feuer":"Fire", "Eis":"Ice", "Sturm":"Storm", "Erde":"Earth", "Licht":"Light", "Schatten":"Shadow"}
EFFECTS = {"projectile":"Projectile", "shield":"Shield", "melee_arc":"MeleeArc", "area":"Area"}

def render():
    spells = json.loads((ROOT / "Production/Magic/spells.json").read_text(encoding="utf-8"))
    if len(spells) != 24 or len({s["id"] for s in spells}) != 24:
        raise ValueError("Exactly 24 unique source spells required")
    rows = []
    for s in spells:
        for field in ("mana_cost", "cooldown_seconds", "power"):
            if not isinstance(s[field], (int, float)) or not math.isfinite(s[field]) or s[field] <= 0:
                raise ValueError("Invalid spell value: " + s["id"] + ":" + field)
        quoted = lambda value: json.dumps(value, ensure_ascii=True)
        rows.append("    {" + ",".join((quoted(s["id"]), quoted(s["name"]), "Element::"+SCHOOLS[s["element"]], "Effect::"+EFFECTS[s["effect"]],str(float(s["mana_cost"])),str(float(s["cooldown_seconds"])),str(float(s["power"]))))+"},")
    return '#pragma once\n// Generated from Production/Magic/spells.json; do not edit by hand.\n#include "Magic/AskirMagicRules.h"\nnamespace AskirMagicRules {\ninline constexpr Spell Catalog[] = {\n'+'\n'.join(rows)+'\n};\ninline constexpr int SpellCount = sizeof(Catalog)/sizeof(Catalog[0]);\nstatic_assert(SpellCount==24);\n}\n'

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("--check",action="store_true");args=parser.parse_args()
    path=ROOT/"Source/Askir/Magic/AskirSpellCatalog.h";expected=render()
    if args.check:
        if not path.is_file() or path.read_text(encoding="utf-8") != expected:raise SystemExit("Catalog is missing or stale")
        print("PASS: 24 runtime spells match canonical source data")
    else:
        path.parent.mkdir(parents=True,exist_ok=True);path.write_text(expected,encoding="utf-8");print("Generated 24 runtime spells")
if __name__=="__main__":main()
