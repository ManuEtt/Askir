#!/usr/bin/env python3
"""Offline world-manifest validation. Does not claim Unreal map generation."""
import argparse,json,pathlib,sys

def validate(path):
    data=json.loads(pathlib.Path(path).read_text(encoding='utf-8'))
    levels=data['levels']; names=[x['map_name'] for x in levels]
    errors=[]
    if len(levels)<30:errors.append('expected at least 30 levels')
    if len(set(names))!=len(names):errors.append('duplicate map names')
    by_name={x['map_name']:x for x in levels}
    for level in levels:
        name=level['map_name']
        if not name.startswith('L') or '/' in name or '..' in name:errors.append(f'invalid map name: {name}')
        for neighbor in level.get('exits',[]):
            if neighbor not in by_name:errors.append(f'{name}: missing exit {neighbor}')
            elif name not in by_name[neighbor].get('exits',[]):errors.append(f'{name}: nonreciprocal exit {neighbor}')
    if names:
        visited={names[0]}; queue=[names[0]]
        while queue:
            for next_name in by_name[queue.pop(0)].get('exits',[]):
                if next_name in by_name and next_name not in visited:visited.add(next_name);queue.append(next_name)
        if len(visited)!=len(names):errors.append('world graph is disconnected')
    return errors

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('manifest');args=parser.parse_args()
    problems=validate(args.manifest)
    for issue in problems:print('ERROR:',issue)
    if problems:sys.exit(1)
    print('PASS: connected world, valid map names, reciprocal links, at least 30 regions')
