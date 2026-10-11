#!/usr/bin/env python3
"""Source-level package sanity check. Not an Unreal compilation."""
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[1]
required={
"V13":["Source/Askir/World/AskirVillageDirector.h","Source/Askir/World/AskirVillageDirector.cpp"],
"V14":["Source/Askir/NPC/AskirInteractionComponent.h","Source/Askir/NPC/AskirInteractionComponent.cpp"],
"V15":["Source/Askir/Save/AskirSaveGame.h"],
"V16":["Source/Askir/Game/AskirGameMode.cpp"],
"V17":["V17_TESTPLAN.md"]
}
errors=[]
for version,paths in required.items():
 for path in paths:
  f=root/path
  if not f.exists() or not f.read_text(encoding="utf8").strip():
   errors.append(f"{version}: missing or empty {path}")
for header in root.glob("Source/Askir/**/*.h"):
 text=header.read_text(encoding="utf8")
 if "UCLASS(" in text and "generated.h" not in text:
  errors.append(f"Missing generated include: {header}")
print("Askir V17 static checks:", "PASS" if not errors else "FAIL")
for e in errors:print(" -",e)
sys.exit(bool(errors))
