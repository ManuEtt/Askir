from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
profiles=json.loads((ROOT/'Production/Bosses/boss_animation_profiles_v60.json').read_text())['profiles']
assert len(profiles)==5
for x in profiles:
 assert x['telegraph_windup_seconds']>=0 and x['stagger_seconds']>=0
 assert x['montage_play_rate']>0
 assert not x['impact_notify_enabled'] or x['attack_montage_asset']
cpp=(ROOT/'Source/Askir/AI/AskirBossAIController.cpp').read_text()
character=(ROOT/'Source/Askir/Characters/AskirCharacter.cpp').read_text()
for name in ['CommitAnimatedAttack','Montage_Play','Montage_Stop','FallbackPendingAttackToTimer','AbortPendingAttack']:
 assert name in cpp+character, name
print('V60 boss animation offline validation PASS')
