from pathlib import Path
import zipfile
root=Path(__file__).resolve().parents[1]
checks={
 "civilian header":root/'Source/Askir/AI/AskirCivilianAIController.h',
 "civilian cpp":root/'Source/Askir/AI/AskirCivilianAIController.cpp',
 "character cpp":root/'Source/Askir/Characters/AskirCharacter.cpp',
 "npc cpp":root/'Source/Askir/Characters/AskirNPC.cpp',
 "release":root/'RELEASE_V21_50_SCHRITTE.md',
 "uproject":root/'AskirRPG.uproject',
}
for name,path in checks.items():
 assert path.is_file(),f"Missing {name}: {path}"
assert 'MoveToActor' in checks['civilian cpp'].read_text()
assert 'OnRoutineChanged.RemoveDynamic' in checks['civilian cpp'].read_text()
assert 'LineTraceSingleByChannel' in checks['character cpp'].read_text()
assert 'FindComponentByClass<UAskirDialogueComponent>' in checks['character cpp'].read_text()
assert checks['release'].read_text().count('- [x]')==50
print('V21 structural checks passed:',len(checks)+5)
