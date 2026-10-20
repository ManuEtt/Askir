"""V81 verification: actual portable gameplay rules plus source-asset checks."""
from pathlib import Path
from datetime import datetime,timezone
import json
import re
import shutil
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]

def run(args):
    r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=120)
    return {'status':'Bestanden' if r.returncode==0 else 'Fehlgeschlagen','command':args,'exit_code':r.returncode,'output':(r.stdout+r.stderr)[-20000:]}

def main():
    from checkpoint_inputs import capture
    before=capture(ROOT)
    checks={
        'world_source_preflight':run([sys.executable,'Tools/build_dornfeld_v80.py','--preflight','--report','Reports/V80/world_source_preflight.json']),
        'delivery_props':run([sys.executable,'Tools/validate_delivery_props_v80.py']),
        'prop_import_inputs':run([sys.executable,'Tools/import_delivery_props_v80.py','--preflight']),
        'village_catalog':run([sys.executable,'Tools/generate_village_catalog_v79.py','--check']),
        'village_source_preflight':run([sys.executable,'Tools/build_dornfeld_v79.py','--preflight','--report','Reports/V79_economy_source_preflight.json']),
        'source_structure':run([sys.executable,'Tools/validate_release.py']),
        'python_regressions':run([sys.executable,'-m','unittest','discover','-s','Tests','-q']),
        'spell_catalog':run([sys.executable,'Tools/generate_spell_catalog_v78.py','--check']),
        'audio_v77':run([sys.executable,'Tools/generate_audio_v77.py','--validate-only']),
        'magic_audio':run([sys.executable,'Tools/generate_magic_audio_v78.py','--validate-only']),
        'fbx_source_inputs':run([sys.executable,'Tools/import_animated_assets_v78.py','--preflight']),
        'actual_skin_and_animation_buffers':run([sys.executable,'Tools/validate_animated_sources_v78.py'])}
    match=re.search(r'Ran (\d+) tests',checks['python_regressions']['output']);checks['python_regressions']['test_count']=int(match.group(1)) if match else None
    compiler=shutil.which('g++')
    for name,source in (('mission_rules','test_dornfeld_rules_v77.cpp'),('checked_arithmetic','test_checked_math_v77.cpp'),('magic_rules','test_magic_rules_v78.cpp'),('economy_rules','test_economy_rules_v79.cpp'),('merchant_state_rules','test_merchant_state_rules_v79.cpp'),('delivery_rules','test_delivery_rules_v80.cpp'),('travel_rules','test_travel_rules_v80.cpp'),('quest_reward_rules','test_quest_reward_rules_v80.cpp'),('settings_rules','test_settings_rules_v81.cpp'),('frontend_rules','test_frontend_rules_v81.cpp')):
        if not compiler:checks[name]={'status':'Blockiert','reason':'No g++ compiler'};continue
        with tempfile.TemporaryDirectory(prefix='askir-v81-') as directory:
            binary=str(Path(directory)/name)
            compile_result=run([compiler,'-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=undefined','-fno-sanitize-recover=all','-I.','-ISource/Askir','Tests/cpp/'+source,'-o',binary])
            checks[name]={'status':compile_result['status'],'compile':compile_result}
            if compile_result['exit_code']==0:checks[name]['execution']=run([binary]);checks[name]['status']=checks[name]['execution']['status']
    from run_unreal_v81 import WORLD
    engine=WORLD.preflight(ROOT/'AskirRPG.uproject',WORLD.discover_engine())
    native=re.findall(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\s*\([^,]+,\s*"([^"]+)"','\n'.join(p.read_text() for p in (ROOT/'Source/Askir/Tests').glob('*.cpp')))
    after=capture(ROOT)
    checks['stable_source_inputs']={'status':'Bestanden' if before==after else 'Fehlgeschlagen','reason':'Source inputs must remain unchanged throughout verification'}
    report={'verified_inputs':after,'version':'V81','world_contract_version':'V80','checked_at':datetime.now(timezone.utc).isoformat(),'checks':checks,'engine_preflight':engine,
            'scope':'Portable production rules and actual source assets; Unreal compilation and gameplay not verified',
            'unreal_tests':{'status':'Blockiert' if engine['issues'] else 'Nicht ausgeführt','registered_tests':sorted(native)},
            'windows_build':{'status':'Blockiert' if engine['issues'] else 'Nicht ausgeführt'},
            'real_maps':len(list((ROOT/'Content').rglob('*.umap'))),'real_unreal_assets':len(list((ROOT/'Content').rglob('*.uasset'))),
            'performance':{'status':'Blockiert','fps':None}}
    (ROOT/'Reports/V81_verification.json').write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8')
    for name,row in checks.items():
        print(name+': '+row['status'])
        if row['status']!='Bestanden':
            details=row.get('execution',row.get('compile',row))
            if details.get('output'):print(details['output'][-4000:],file=sys.stderr)
    print('Unreal/Win64: '+report['windows_build']['status'])
    return int(any(row['status']!='Bestanden' for row in checks.values()))
if __name__=='__main__':raise SystemExit(main())
