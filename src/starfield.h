#pragma once
#include <stdint.h>

/*
 * starfield.h — static 3D background stars for the rails stage.
 *
 * Tiny vertex-colored quads scattered through world space across the rail's
 * Z range, filled once and replayed every frame (they are fixed world objects,
 * not updated per frame).  Seed-derived placement.  The caller sets
 * T3D_FLAG_NO_LIGHT before drawing so stars glow at full vertex color.
 *
 * Note: this uses its own LCG for placement, separate from the xorshift32 in
 * rng.h — kept distinct so the star pattern is stable independent of the
 * gameplay generators.
 *
 * LIFECYCLE
 *   starfield_init(railStart, railEnd); // once — allocate buffers, set Z range
 *   starfield_generate(seed);           // on stage entry — scatter the stars
 *   starfield_draw();                   // each frame — replay the star geometry
 */

/* Allocate buffers and record the rail Z range stars span. Call once. */
void starfield_init(float railStart, float railEnd);

/* Scatter the stars deterministically from the seed. */
void starfield_generate(uint32_t seed);

/* Draw the full star field (batched to fit the RSP vertex cache). */
void starfield_draw(void);
