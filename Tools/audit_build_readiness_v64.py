"""Offline project readiness checks; does not compile Unreal C++."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parent.parent

def audit():
    errors=[]
    def require(path):
        p=ROOT/path
        if not p.is_file():errors.append(f"Missing: {path}")
        return p
    u=require('AskirRPG.uproject')
    if u.is_file():
        data=json.loads(u.read_text(encoding='utf-8'))
        if data.get('EngineAssociation')!='5.6':errors.append('Expected Unreal 5.6')
    require('Source/Askir/Askir.Build.cs')
    require('Source/Askir/AI/AskirBossAIController.cpp')
    require('Tools/setup_boss_ai_v56.py')
    for f in (ROOT/'Production').rglob('*.json'):
        try:json.loads(f.read_text(encoding='utf-8'))
        except (ValueError,UnicodeError) as e:errors.append(f"Invalid JSON: {f.relative_to(ROOT)}: {e}")
    for f in (ROOT/'Tools').glob('*.py'):
        try:compile(f.read_text(encoding='utf-8'),str(f),'exec')
        except SyntaxError as e:errors.append(f"Syntax: {f.name}: {e}")
    return errors

if __name__=='__main__':
    errors=audit()
    print('\n'.join(errors) if errors else 'PASS: offline build-readiness audit (not Unreal compilation)')
    sys.exit(bool(errors))
