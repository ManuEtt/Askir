from pathlib import Path
root=Path(__file__).resolve().parents[1]
checks={
 'Unreal project':(root/'AskirRPG.uproject').is_file(),
 'Encounter zone':(root/'Source/Askir/World/AskirEncounterZone.cpp').is_file(),
 'Wave spawner':(root/'Source/Askir/World/AskirEncounterSpawner.cpp').is_file(),
 'Wave header':(root/'Source/Askir/World/AskirEncounterSpawner.h').is_file(),
 'Quest progress': 'AdvanceQuestObjective' in (root/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text(),
 'Death binding': 'OnDeath.AddDynamic' in (root/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text(),
 'Collision fallback': 'AdjustIfPossibleButDontSpawnIfColliding' in (root/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text(),
 'Wave bounds': 'Clamp(WaveCount,1,10)' in (root/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text(),
 'GitHub attributes':(root/'.gitattributes').exists(),
 'Asset import':(root/'Tools/import_enhanced_fbx_unreal.py').exists(),
}
for k,v in checks.items():print(('PASS' if v else 'FAIL'),k)
assert all(checks.values())
