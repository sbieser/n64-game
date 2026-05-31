"""
Generate assets/signal.wav — the Pioneer beacon for Stage 1.

Two tones a perfect fourth apart (168 Hz and 224 Hz, ratio 4:3).  The fourth
is one of the oldest musical intervals: resolved enough to feel present,
unresolved enough to feel like a question.  Their beating creates a ~56 Hz
difference frequency felt as low physical presence.

Each tone drifts independently in pitch (slow FM) so they never lock into a
clean harmonic.  A breathing envelope — the product of three incommensurable
slow sine waves — ensures the signal never reads as mechanical.  It swells
and recedes like something alive, or once-alive.

Fade-in / fade-out at the loop boundary make the seam inaudible.
"""

import struct
import math

RATE     = 22050
DURATION = 8.0
SAMPLES  = int(RATE * DURATION)

F1        = 168.0          # deep root tone
F2        = 224.0          # perfect fourth (4:3)

FM1_RATE  = 0.09           # Hz — pitch drift rate, tone 1
FM1_DEPTH = 8.0            # Hz — pitch drift depth, tone 1
FM2_RATE  = 0.17           # Hz — different rate, tones drift independently
FM2_DEPTH = 6.0            # Hz

ENV_RATES = [0.07, 0.11, 0.19]   # Hz — incommensurable breathing rates

FADE_IN   = int(0.15 * RATE)
FADE_OUT  = int(0.25 * RATE)

buf    = []
phase1 = 0.0
phase2 = 0.0

for i in range(SAMPLES):
    t = i / RATE

    freq1 = F1 + FM1_DEPTH * math.sin(2 * math.pi * FM1_RATE * t)
    freq2 = F2 + FM2_DEPTH * math.sin(2 * math.pi * FM2_RATE * t)

    phase1 += 2 * math.pi * freq1 / RATE
    phase2 += 2 * math.pi * freq2 / RATE

    tone1 = math.sin(phase1)
    tone2 = math.sin(phase2) * 0.65   # second tone slightly quieter

    # Breathing envelope: product of three slow sines → long irregular swells
    env = 1.0
    for rate in ENV_RATES:
        env *= 0.5 + 0.5 * math.sin(2 * math.pi * rate * t)
    env = env ** 0.6          # soften troughs — silences feel like held breath, not cut off
    env = 0.10 + 0.90 * env   # noise floor — never fully silent

    # Fade at loop boundary so the seam is inaudible
    if i < FADE_IN:
        env *= i / FADE_IN
    elif i > SAMPLES - FADE_OUT:
        env *= (SAMPLES - i) / FADE_OUT

    sample = (tone1 + tone2) * env * 0.42   # keep peak below 1.0

    s16 = int(sample * 28000)
    s16 = max(-32768, min(32767, s16))
    buf.append(s16)

data_size = SAMPLES * 2
with open('assets/signal.wav', 'wb') as f:
    f.write(b'RIFF')
    f.write(struct.pack('<I', 36 + data_size))
    f.write(b'WAVE')
    f.write(b'fmt ')
    f.write(struct.pack('<I', 16))
    f.write(struct.pack('<H', 1))            # PCM
    f.write(struct.pack('<H', 1))            # mono
    f.write(struct.pack('<I', RATE))
    f.write(struct.pack('<I', RATE * 2))     # byte rate
    f.write(struct.pack('<H', 2))            # block align
    f.write(struct.pack('<H', 16))           # bits per sample
    f.write(b'data')
    f.write(struct.pack('<I', data_size))
    for s in buf:
        f.write(struct.pack('<h', s))

print(f"Generated assets/signal.wav  ({DURATION}s, {RATE} Hz, mono, {SAMPLES} samples)")
