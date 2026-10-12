"""Offline cross-check of Unreal C++ reflection names and editor actor assignments."""
import json
import re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools/build_dornfeld_playground_v74.py"
HEADERS = {
 "AskirNPC": ROOT / "Source/Askir/Characters/AskirNPC.h",
 "AskirQuestTrigger": ROOT / "Source/Askir/Economy/AskirQuestTrigger.h",
 "AskirLootChest": ROOT / "Source/Askir/Loot/AskirLootChest.h",
}
FIELDS = {
 "AskirNPC": {"npc_name": "NPCName", "b_hostile": "bHostile", "defeat_objective_id": "DefeatObjectiveID"},
 "AskirQuestTrigger": {"quest_id": "QuestID", "objective_id": "ObjectiveID", "amount": "Amount"},
 "AskirLootChest": {"persistent_loot_id": "PersistentLootID", "quest_id": "QuestID", "objective_id": "ObjectiveID"},
}
def validate():
    script = SCRIPT.read_text(encoding="utf-8")
    issues=[]
    for cls, mapping in FIELDS.items():
        header=HEADERS[cls].read_text(encoding="utf-8")
        for editor_name, cpp_name in mapping.items():
            if not re.search(r"\b"+re.escape(cpp_name)+r"\b",header): issues.append(f"{cls}.{cpp_name} absent")
            if not re.search(r"set_editor_property\(['\"]"+re.escape(editor_name)+r"['\"]",script): issues.append(f"{cls}.{editor_name} not assigned")
    quest=json.loads((ROOT/'Production/Quests/dornfeld_first_playable_mission.json').read_text(encoding='utf-8'))
    if quest['id']!='DORNFELD_BRUNNEN': issues.append('quest ID changed')
    for stage in ('COLLECT_SHARDS','DEFEAT_GUARDIANS','OPEN_CHEST'):
        if stage not in [x['id'] for x in quest['stages']] or stage not in script: issues.append('missing stage '+stage)
    return issues
if __name__=='__main__':
    errors=validate()
    print(json.dumps({'status':'PASS' if not errors else 'FAIL','issues':errors,'scope':'offline source contract only'},indent=2))
    raise SystemExit(bool(errors))
