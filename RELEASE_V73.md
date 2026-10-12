# ASKIR RPG V73 — Dornfeld mission contract regression

Base: V72. Added `Tools/dornfeld_mission_preflight_v73.py`, five regression tests, and a machine-readable preflight report.

Verified locally: 418 Python unit tests passed; Dornfeld quest contract preserves five ordered stages, Mara, three shards, two guardians, and 90 gold / 180 XP.

Not verified: Unreal C++ compilation, `.umap` creation, Blueprint compilation, in-editor playability, SaveGame persistence, Windows packaging. No claims of playable content are made by this release.

Windows handoff: open `.uproject` with Unreal Engine 5.6, compile the Askir module, generate/import levels with the existing Tools scripts, run in-editor automation and gameplay tests. Existing scripts may require explicit dry-run settings and editor APIs.
