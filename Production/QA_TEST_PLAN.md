# QA scenarios

| ID | Scenario | Expected |
|---|---|---|
| QA-01 | Launch village | Player spawns and camera works |
| QA-02 | Enhanced Input move/look | Movement axes correctly mapped |
| QA-03 | Talk to quest giver | Dialogue starts and closes |
| QA-04 | Accept SQ_001 | Quest state active |
| QA-05 | Trigger quest event | Objective increments only once per event |
| QA-06 | Claim reward twice | Second claim rejected |
| QA-07 | Save then reload | Gold, quest states, inventory restored |
| QA-08 | Block and dodge | Stamina consumed and i-frames honored |
| QA-09 | Kill enemy | XP and loot granted once |
| QA-10 | Spell cooldown | Immediate recast rejected |
| QA-11 | Craft without materials | No item produced |
| QA-12 | Craft with materials | Ingredients deducted atomically |
| QA-13 | Trader buy/sell | Prices and stock validated |
| QA-14 | NPC navigation | NavMesh path found |
| QA-15 | Boss defeat | Dungeon quest objective updated |
| QA-16 | All 5 maps | Travel and spawn point valid |
| QA-17 | Localization | German UI text displays correctly |
| QA-18 | Corrupted save | Safe failure, no crash |
| QA-19 | Missing DataTable row | Graceful false result |
| QA-20 | Windows package | Builds, launches, saves |

All gameplay scenarios are **NOT RUN**; require Unreal Engine 5.6.
