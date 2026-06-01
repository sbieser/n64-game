#pragma once
#include <stdint.h>

/*
 * beacon.h — Stage 1 Layer 5: the Pioneer's expanding ring pulse.
 *
 * A very slow, dim, expanding ring centered on the Pioneer — a ripple on an
 * invisible pond, drifting outward and fading. Deliberately NOT a clean sonar
 * ping: it lies in a fixed world plane (so it reads as an oblique ellipse from
 * the player's approach, not a halo around the target), it is nearly invisible,
 * and its timing is irregular. It should provoke "...what is that?", not
 * "follow me." The discovery is in the noticing.
 *
 * A small pool of rings cycle independently with random gaps (seeded), so
 * pulses overlap organically instead of ticking like a metronome.
 *
 * USAGE
 *   beacon_init(seed, cx, cy, cz);  // once — center on the Pioneer
 *   beacon_update();                // each frame — advance the ring timers
 *   beacon_draw();                  // each frame, in the 3D pass near the Pioneer
 */

void beacon_init(uint32_t seed, float cx, float cy, float cz);
void beacon_update(void);
void beacon_draw(void);
