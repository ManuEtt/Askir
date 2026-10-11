"""Offline V49 contract checks for editor integration, independent of Unreal."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
profiles=json.loads((ROOT/'Production/Levels/encounter_profiles_v48.json').read_text(encoding='utf-8'))['profiles']
world=json.loads((ROOT/'Production/Levels/world_gameplay_anchors_v46.json').read_text(encoding='utf-8'))
assert len(profiles)==30
encounters=[(p['map_name'],e) for p in profiles for e in p['encounters']]
assert len(encounters)==90
assert len({e['encounter_id'] for _,e in encounters})==90
assert all(e['enemy_count']>=1 and e['enemy_count']<=32 for _,e in encounters)
assert all(len(e['position_cm'])==3 for _,e in encounters)
assert all(e['blueprint_class_path'] is None or isinstance(e['blueprint_class_path'],str) for _,e in encounters)
source=(ROOT/'Tools/integrate_encounter_actors_v49.py').read_text(encoding='utf-8')
for token in ['DRY_RUN = True','V49_Zone_','V49_Spawner_','AskirEncounterZone','AskirEncounterSpawner','save_current_level','does_asset_exist','set_editor_property']:
 assert token in source,token
print('V49 offline contract OK: 30 maps, 90 encounters, actor integration script.')
