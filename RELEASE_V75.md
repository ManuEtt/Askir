# ASKIR RPG V75 — Dornfeld editor-to-C++ quest contract

## Implemented
- Corrected quest/objective assignments in the opt-in UE Editor Python playground generator for shard triggers and cellar chest.
- Aligned shadow guardian objective IDs with the existing Dornfeld mission data.
- Added `Tools/audit_dornfeld_editor_contract_v75.py` to verify assigned Unreal property names against C++ headers and objective IDs against quest data.
- Added three regression tests in `Tests/test_dornfeld_contract_v75.py`.

## Verification
- `python -m unittest discover -s Tests -q`: 427 tests passed.
- `python Tools/audit_dornfeld_editor_contract_v75.py`: PASS.
- Unreal Editor execution: NOT RUN. C++ compilation: NOT RUN. Windows packaged build: NOT RUN.

## Known limitations
- The editor generator remains preview-only by default; no `.umap` is created here.
- The quest trigger uses overlaps, not inventory pickups. A complete three-distinct-shards collection system, acceptance dialogue, navigation, chest contents, save/load and reward turn-in still require integration and in-engine validation.
- Existing map is never overwritten by the generator.
