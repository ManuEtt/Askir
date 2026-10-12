#!/usr/bin/env python3
"""Generate original ASKIR procedural PCM audio; no samples or dependencies.

Run: python Tools/generate_audio_v77.py
Unreal import/playback are separate steps and are not claimed by this generator.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import random
import struct
import wave

RATE = 48_000
TAU = math.tau
ROOT = Path(__file__).resolve().parents[1]
SEED = 77_010


def bell(t: float, frequency: float, decay: float = 5.0) -> float:
    if t < 0.0:
        return 0.0
    attack = min(1.0, t / 0.008)
    return attack * math.exp(-decay * t) * (
        math.sin(TAU * frequency * t)
        + 0.36 * math.sin(TAU * frequency * 2.01 * t)
        + 0.13 * math.sin(TAU * frequency * 3.96 * t)
    )


def synthesize(cue: str, duration: float) -> list[float]:
    rng = random.Random(SEED + sum((i + 1) * ord(c) for i, c in enumerate(cue)))
    samples: list[float] = []
    filtered = 0.0
    for index in range(round(duration * RATE)):
        t = index / RATE
        noise = rng.uniform(-1.0, 1.0)
        previous = filtered
        filtered = 0.88 * filtered + 0.12 * noise
        if cue == 'Shard':
            value = bell(t, 880.0, 7.0) + 0.48 * bell(t - 0.10, 1320.0, 8.0) + 0.18 * bell(t - 0.17, 1760.0, 9.0)
        elif cue == 'Chest':
            creak = math.sin(TAU * (118.0 * t + 16.0 * math.sin(6.0 * t)))
            movement = math.sin(math.pi * min(1.0, t / 0.92)) ** 2
            hinge = 0.23 * creak * movement * math.exp(-1.2 * t)
            latch_t = t - 0.82
            latch = (0.7 * noise + math.sin(TAU * 310.0 * latch_t)) * math.exp(-45.0 * latch_t) if latch_t >= 0 else 0.0
            value = hinge + 0.48 * filtered * movement + 0.23 * math.sin(TAU * 62.0 * t) * math.exp(-12.0 * t) + latch
        elif cue == 'QuestAccepted':
            value = sum(0.7 * bell(t - onset, note, 4.3) for onset, note in (
                (0.0, 261.63), (0.17, 311.13), (0.36, 392.0), (0.58, 523.25)
            ))
        elif cue == 'QuestCompleted':
            value = sum(0.55 * bell(t - onset, note, 3.1) for onset, note in (
                (0.0, 293.66), (0.20, 349.23), (0.42, 440.0), (0.69, 587.33), (0.98, 698.46)
            )) + 0.16 * bell(t - 0.98, 880.0, 3.0)
        elif cue == 'MeleeSwing':
            motion = math.sin(math.pi * t / duration) ** 1.6
            high_noise = noise - previous
            value = (0.72 * filtered + 0.16 * high_noise) * motion * (0.6 + 0.4 * t / duration)
        elif cue == 'MeleeHit':
            pitch_phase = TAU * (46.0 * t + 145.0 * (1.0 - math.exp(-18.0 * t)) / 18.0)
            thump = math.sin(pitch_phase) * math.exp(-13.0 * t)
            crack = (noise - previous) * math.exp(-48.0 * t)
            body = filtered * math.exp(-9.0 * t)
            value = 0.65 * thump + 0.24 * crack + 0.22 * body
        elif cue == 'DungeonAmbient':
            # Every oscillator/modulator completes an integer number of cycles over
            # eight seconds. The discrete loop boundary has no discontinuity.
            phase = TAU * t / duration
            breath = 0.74 + 0.20 * math.cos(phase) + 0.06 * math.cos(3.0 * phase)
            drone = (
                0.55 * math.sin(251.0 * phase + 0.4)
                + 0.28 * math.sin(375.0 * phase + 0.9)
                + 0.17 * math.sin(502.0 * phase + 1.4)
                + 0.10 * math.sin(751.0 * phase + 0.2)
            )
            air = sum(0.018 * math.sin(cycles * phase + offset) for cycles, offset in (
                (997, 0.2), (1231, 1.7), (1693, 2.2), (2111, 0.8), (2879, 2.9)
            ))
            value = breath * drone + air * (0.8 + 0.2 * math.cos(2.0 * phase))
        else:
            raise ValueError('Unknown cue: ' + cue)
        if cue != 'DungeonAmbient':
            fade = min(1.0, t / 0.005, max(0.0, (duration - t - 1.0 / RATE) / 0.025))
            value *= fade
        samples.append(value)
    peak = max(abs(value) for value in samples)
    gain = (0.34 if cue == 'DungeonAmbient' else 0.82) / peak
    return [value * gain for value in samples]


ASSETS = [
    ('Shard', 0.85, False, 'Original crystal pickup chime'),
    ('Chest', 1.25, False, 'Original wooden hinge and latch effect'),
    ('QuestAccepted', 1.30, False, 'Original four-note acceptance motif'),
    ('QuestCompleted', 2.05, False, 'Original completion motif'),
    ('MeleeSwing', 0.40, False, 'Original synthesized weapon whoosh'),
    ('MeleeHit', 0.55, False, 'Original low impact and crack effect'),
    ('DungeonAmbient', 8.00, True, 'Original seamless low drone and air loop'),
]


def inspect(path: Path, expected_duration: float, looping: bool) -> dict:
    with wave.open(str(path), 'rb') as source:
        channels, width, rate, frames = source.getnchannels(), source.getsampwidth(), source.getframerate(), source.getnframes()
        compression = source.getcomptype()
        raw = source.readframes(frames)
    if (channels, width, rate, compression) != (1, 2, RATE, 'NONE'):
        raise ValueError('Invalid PCM header: ' + str(path))
    if frames != round(expected_duration * RATE) or len(raw) != frames * 2:
        raise ValueError('Invalid frame count: ' + str(path))
    values = struct.unpack('<' + 'h' * frames, raw)
    peak = max(abs(value) for value in values) / 32768.0
    rms = math.sqrt(sum((value / 32768.0) ** 2 for value in values) / frames)
    if not 0.001 < rms < 0.6 or peak >= 0.99:
        raise ValueError('Silence/clipping check failed: ' + str(path))
    seam = abs(values[0] - values[-1]) / 32768.0
    if looping and seam > 0.01:
        raise ValueError('Loop boundary discontinuity: ' + str(path))
    if not looping and (values[0] != 0 or values[-1] != 0):
        raise ValueError('Transient endpoints must be zero: ' + str(path))
    return {
        'sample_rate_hz': rate, 'channels': channels, 'pcm_bits': width * 8,
        'frames': frames, 'duration_seconds': frames / rate,
        'rms': round(rms, 8), 'peak': round(peak, 8),
        'boundary_step': round(seam, 8),
        'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
        'validation': 'PASSED',
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'ArtSource' / 'Audio_V77')
    parser.add_argument('--validate-only', action='store_true')
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    records = []
    for cue, duration, looping, description in ASSETS:
        name = 'S_' + cue + '.wav'
        path = output / name
        if not args.validate_only:
            samples = synthesize(cue, duration)
            pcm = struct.pack('<' + 'h' * len(samples), *(round(value * 32767.0) for value in samples))
            with wave.open(str(path), 'wb') as target:
                target.setnchannels(1)
                target.setsampwidth(2)
                target.setframerate(RATE)
                target.writeframes(pcm)
        record = {'cue': cue, 'file': name, 'looping': looping, 'description': description}
        record.update(inspect(path, duration, looping))
        records.append(record)
    if len({record['sha256'] for record in records}) != len(records):
        raise ValueError('Audio cues are not distinct')
    manifest = {
        'version': 77,
        'creator': 'ASKIR development: original procedural synthesis',
        'license': 'CC0-1.0',
        'license_url': 'https://creativecommons.org/publicdomain/zero/1.0/',
        'external_samples': [],
        'composition': 'Original algorithmic effects and motifs; no imported recordings, samples, or music.',
        'generator': 'Tools/generate_audio_v77.py',
        'generator_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        'seed': SEED,
        'unreal_import': 'NOT_EXECUTED',
        'unreal_playback': 'NOT_EXECUTED',
        'assets': records,
    }
    if not args.validate_only:
        (output / 'audio_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
        (output / 'LICENSE.txt').write_text(
            'ASKIR V77 original procedural audio\n\n'
            'The audio files in this directory are dedicated to the public domain under CC0 1.0.\n'
            'No external recordings, samples, compositions, or licensed libraries were used.\n'
            'https://creativecommons.org/publicdomain/zero/1.0/\n', encoding='utf-8')
    print(json.dumps({'assets': len(records), 'validation': 'PASSED', 'total_seconds': sum(r['duration_seconds'] for r in records),
                      'output': str(output), 'unreal_import': 'NOT_EXECUTED'}, indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
