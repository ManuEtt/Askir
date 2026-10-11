# Askir V56 — Boss-Phasen (Themen 1551–1600)

Cumulative Unreal Engine 5.6 source project based on V55. New native `AAskirBossAIController` tracks health thresholds (66%, 33%), permanently transitions through three phases, scales attack interval/range, and exposes `OnBossPhaseChanged` for future Blueprint animations and VFX. Boss phase events do not automatically play animations.

## Integration
1. Compile C++ in UE 5.6 and verify the native class.
2. Generate missing Blueprint assets with V51 script.
3. Run `Tools/setup_boss_ai_v56.py` with `DRY_RUN=True`; inspect logs, back up Content, then set `False` and rerun.
4. Inspect boss Blueprint AI Controller Class and Auto Possess AI after reopening.
5. Implement `OnBossPhaseChanged` in boss AI Blueprint, add actual montages/VFX, and test thresholds and attacks in PIE.

## Offline tests
`python -m unittest discover -s Tests -p 'test_boss_phases_v56.py' -v`

**Limitations:** No UE compiler/editor available; Blueprint CDO persistence not verified. No animations, sounds, VFX or playable build claimed. All previous project source files retained.
