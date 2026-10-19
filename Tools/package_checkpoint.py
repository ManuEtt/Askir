#!/usr/bin/env python3
"""Create and byte-verify a source checkpoint ZIP, never an invented Windows build."""
from pathlib import Path
from datetime import datetime,timezone
import argparse
import hashlib
import json
import re
import subprocess
import zipfile

ROOT=Path(__file__).resolve().parents[1]

def build(version,output,verification):
    if not re.fullmatch(r'V[0-9]+(?:_[A-Za-z0-9]+)*',version):raise ValueError('Use a version such as V78 or V78_Checkpoint01')
    output=Path(output).resolve()
    if output.is_relative_to(ROOT):raise ValueError('Keep archives outside the project to prevent recursive packages')
    report=json.loads(Path(verification).read_text(encoding='utf-8'))
    if report.get('version')!=version.split('_',1)[0]:raise ValueError('Verification version must match the checkpoint')
    if any(check.get('status')!='Bestanden' for check in report['checks'].values()):raise ValueError('Offline checks must pass before checkpoint packaging')
    if int(version.split('_',1)[0][1:]) >= 79:
        from checkpoint_inputs import capture
        if not report.get('verified_inputs') or report['verified_inputs'] != capture(ROOT):
            raise ValueError('Source inputs changed since verification; run the verifier again')
    paths=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
    paths=sorted({name for name in paths if name and (ROOT/name).is_file()})
    rows=[]
    for name in paths:
        source=ROOT/name
        if source.is_symlink() or not source.resolve().is_relative_to(ROOT):raise ValueError('Unsafe project source path: '+name)
        if source.read_bytes().startswith(b'version https://git-lfs.github.com/spec/v1\n'):raise ValueError('LFS object not materialized: '+name)
        rows.append({'path':name,'bytes':source.stat().st_size,'sha256':hashlib.sha256(source.read_bytes()).hexdigest()})
    manifest={'version':version,'created_at':datetime.now(timezone.utc).isoformat(),'kind':'Unreal development source checkpoint',
              'git_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
              'working_tree_dirty':bool(subprocess.check_output(['git','status','--porcelain'],cwd=ROOT,text=True).strip()),
              'verification_report':str(Path(verification).resolve().relative_to(ROOT)),
              'unreal_playability':'Nicht ausgeführt','windows_build':'Blockiert','files':rows}
    prefix='AskirRPG_'+version+'/';output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
        for row in rows:archive.write(ROOT/row['path'],prefix+row['path'])
        archive.writestr(prefix+'DELIVERY_MANIFEST.json',json.dumps(manifest,indent=2,ensure_ascii=False))
    with zipfile.ZipFile(output) as archive:
        if archive.testzip() is not None:raise RuntimeError('ZIP CRC validation failed')
        if set(archive.namelist())!={prefix+r['path'] for r in rows}|{prefix+'DELIVERY_MANIFEST.json'}:raise RuntimeError('ZIP file set mismatch')
        for row in rows:
            if hashlib.sha256(archive.read(prefix+row['path'])).hexdigest()!=row['sha256']:raise RuntimeError('ZIP source mismatch: '+row['path'])
    digest=hashlib.sha256(output.read_bytes()).hexdigest();output.with_suffix(output.suffix+'.sha256').write_text(digest+'  '+output.name+'\n')
    return {'zip':str(output),'bytes':output.stat().st_size,'entries':len(rows)+1,'sha256':digest,'crc':'Bestanden','all_file_hashes':'Bestanden'}

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--version',required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verification',type=Path,required=True);args=parser.parse_args()
    print(json.dumps(build(args.version,args.output,args.verification),indent=2,ensure_ascii=False))
if __name__=='__main__':main()
