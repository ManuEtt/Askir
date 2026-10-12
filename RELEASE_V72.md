# ASKIR RPG V72 — Campaign graph integrity

## Implemented
- Added `Tools/validate_campaign_graph_v72.py` to validate the canonical 30-region manifest offline.
- Checks duplicate map names, missing destinations, self-links, duplicate exits, reverse transitions, mandatory metadata, and reachability from Dornfeld.
- Added four automated regression tests in `Tests/test_campaign_graph_v72.py`.
- Wrote machine-readable report `Production/Levels/campaign_graph_audit_v72.json`.

## Verified on 2026-10-12
- `python Tools/validate_campaign_graph_v72.py`: PASS; 30/30 regions reachable, zero graph errors and warnings.
- `python -m unittest discover -s Tests -p 'test_*.py' -q`: PASS; 413 tests.

## Not verified
- No Unreal Engine editor/compiler is available in this execution environment. Manifest entries are NOT `.umap` levels; map creation, Blueprint compilation, end-to-end playability and Windows packaging remain unverified.
- No GitHub push was performed.

## Next Windows validation
1. Back up project, generate blockout maps using existing editor script after C++ compilation.
2. Inspect each generated `.umap` and verify transitions and actual gameplay actors.
3. Execute Unreal Automation Tests and package Win64 build.
