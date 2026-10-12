# ASKIR RPG V76 – persistent unique quest pickups

- Added `ConsumedQuestTriggers` as a `SaveGame` property on the quest component.
- Added `PersistentTriggerID` to quest trigger actors. A consumed trigger will not advance a quest again, even after a level reload, through the explicit `UAskirSaveGame` snapshot fields.
- Dornfeld editor generator assigns three distinct stable IDs to the three shard triggers.
- Added three offline regression checks.

## Limitations
- Unreal C++ compilation and editor execution were not possible in this environment.
- The custom GameMode save/load snapshot now copies the consumed-trigger set both ways; this is statically tested but not engine-tested.
- This does not yet implement quest acceptance dialogue, a full combat loop, chest rewards or a playable alpha.
