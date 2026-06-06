#pragma once
#include <stdint.h>

/*
 * debris.h — Stage 1 navigational debris field (rocks, chunks, wreckage).
 *
 * The void is enormous and a giant EMPTY void is not "lonely", it is
 * disorienting and unplayable: no landmarks, nothing to fly toward, no way to
 * tell if you are making progress. This field fixes that by filling the void
 * with physical objects whose DENSITY carries information.
 *
 * THREE TIERS
 * ───────────
 *   1. Large landmarks  — a few big chunks visible across the local space;
 *                         fixed reference points to orient by and aim at.
 *   2. Uniform scatter  — a thin baseline everywhere, so you are never in pure
 *                         black with nothing to fly to.
 *   3. Gradient cluster — density rises sharply toward the Pioneer. Following
 *                         the thickening debris leads the player inward, into
 *                         the signal's detection radius. The debris is the
 *                         long-range guide that the (silent-until-close) signal
 *                         cannot be.
 *
 * Each piece reuses a shapes.c mesh at a static, randomly-oriented world
 * matrix built once at init (landmarks don't drift). Drawing culls everything
 * beyond the scene far-clip, so only the local neighborhood is ever emitted.
 *
 * USAGE
 *   debris_init(seed, pioneerX, pioneerY, pioneerZ);  // once
 *   debris_draw(camX, camY, camZ);                    // each frame, 3D pass
 */

void debris_init(uint32_t seed, float pioneerX, float pioneerY, float pioneerZ);
void debris_draw(float camX, float camY, float camZ);
