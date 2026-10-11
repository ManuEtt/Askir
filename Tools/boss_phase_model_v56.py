"""Offline reference for health-driven phase transitions (no Unreal dependency)."""
def next_phase(current, health, phase_two=.66, phase_three=.33):
    if not 1 <= current <= 3:
        raise ValueError("current phase must be 1..3")
    if not 0 <= health <= 1:
        raise ValueError("health fraction must be 0..1")
    if not 0 < phase_three <= phase_two < 1:
        raise ValueError("phase thresholds must be ordered")
    desired = 3 if health <= phase_three else 2 if health <= phase_two else 1
    return max(current, desired)

def attack_interval(base_interval, speed_multiplier):
    if base_interval <= 0 or speed_multiplier <= 0:
        raise ValueError("interval and multiplier must be positive")
    return base_interval / speed_multiplier
