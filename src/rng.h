#pragma once
#include <stdint.h>

/*
 * rng.h — xorshift32, the canonical deterministic PRNG for procedural content.
 *
 * WHY THIS EXISTS
 * ───────────────
 * The whole game is built on one idea: a single uint32_t seed reproduces an
 * entire run.  Obstacle layouts, gravity fields, star density — every random
 * choice is a draw from this generator.  Store the seed and you can replay the
 * run exactly; that is what makes ghost records and compact saves possible.
 *
 * For that promise to hold, every system MUST draw from the same generator in
 * a fixed, documented order.  Three files used to carry their own private copy
 * of the algorithm; this module is the single source of truth so they can never
 * silently drift apart.
 *
 * HOW XORSHIFT32 WORKS
 * ────────────────────
 *   state ^= state << 13
 *   state ^= state >> 17
 *   state ^= state << 5
 *
 * Each call mixes the bits of a 32-bit integer with shifted copies of itself.
 * The shift trio (13, 17, 5) is chosen so the generator visits all 2^32 − 1
 * non-zero states before repeating — a "full period".  It is fast (no
 * multiply), cheap, and statistically good enough for game content.  It is NOT
 * cryptographic, and it does not need to be.
 *
 * One important property: it is chaotic.  Seed 1000 and seed 1001 produce
 * completely unrelated sequences — adjacent seeds give unrecognisably different
 * runs.  (Never seed with 0: state 0 maps to 0 forever.)
 *
 * USAGE
 * ─────
 *   Rng rng;
 *   rng_seed(&rng, seed);
 *   uint32_t v = rng_next(&rng);      // raw 32-bit draw
 *   int      t = rng_range(&rng, 5);  // 0..4
 *   float    u = rng_unit(&rng);      // 0.0 .. ~1.0
 *
 * The Rng state is a plain struct held by the caller (not a global), so two
 * independent streams can coexist — e.g. obstacles and gravity fields each
 * seed their own Rng, often from the same seed XOR'd with a domain constant
 * so the two streams don't march in lockstep.
 */

typedef struct {
    uint32_t state;
} Rng;

/* Initialise the generator. Seed 0 is remapped to 1 (0 is a dead state). */
void rng_seed(Rng *rng, uint32_t seed);

/* Advance the state and return the raw 32-bit value. */
uint32_t rng_next(Rng *rng);

/* Uniform integer in [0, n).  Requires n > 0. */
uint32_t rng_range(Rng *rng, uint32_t n);

/* Uniform float in [0, 1) using a 1/10000 quantisation (matches the historical
 * `(rng_next() % 10000) / 10000.0f` idiom used across the generators). */
float rng_unit(Rng *rng);
