"""Run INSIDE UE5.6 Python editor after C++ compilation. Never edits maps by default."""
import os,runpy,unreal
ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DRY_RUN=True
STAGES=['generate_30_world_blockouts_v36.py','setup_world_portals_v43.py','integrate_30_level_gameplay_v44.py']
if not DRY_RUN:
    raise RuntimeError('Execute each stage separately after backups and set DRY_RUN=False in the individual script; automatic writes are intentionally disabled.')
for script in STAGES:
    unreal.log('V45: inspect and execute manually: '+os.path.join(ROOT,'Tools',script))
unreal.log('V45: before editing, run python Tools/preflight_world_v45.py outside Unreal')
