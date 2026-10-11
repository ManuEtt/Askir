# Askir V57 – Boss feedback events and encounter profiles

Cumulative UE 5.6 source project based on V56. Adds a native multicast phase transition delegate (`OnPhaseTransition`), optional Blueprint attack telegraph hook, cooldown-protected callable telegraph trigger, and five authored boss encounter tuning profiles. The telegraph is **not automatically triggered by attacks**; gameplay/animation Blueprint must call `TriggerAttackTelegraph()` at an appropriate time. It does not delay damage by itself. Five boss profiles are data-only and not spawned automatically.

## Validation
`python -m unittest discover -s Tests -p test_boss_events_v57.py -v`

## Unreal steps
Compile C++; assign boss AI controller; bind `OnPhaseTransition` for HUD/music; implement `OnBossAttackTelegraph` for warning VFX; connect the telegraph call to attack decision/animation notify in UE. Test in PIE. No Unreal build or PIE test performed here.
