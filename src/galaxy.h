#pragma once
#include <stdint.h>

/*
 * galaxy.h — a procedural spiral galaxy (2D, angle-locked landmark).
 *
 * Where the nebula washes are soft ambient color, a galaxy is a structured
 * landmark: a bright warm core with logarithmic spiral arms, built from a few
 * hundred small colored points. Color grades by radius: warm core → blue-white
 * arms → faint magenta edges. The structure is in the generated point layout,
 * not a texture (see NOTES.md "Galaxies: Two Roads").
 *
 * 2D screen-space, using the same angle scroll as the star carpet (skyproj.h),
 * so it locks to the stars and moves as one sky. It sits at a fixed sky
 * direction and is drawn on top of the carpet/nebula as crisp points.
 *
 * USAGE
 *   galaxy_init(seed);          // once — generate the disc
 *   galaxy_draw(yaw, pitch);    // each frame, on top of the carpet and nebula
 */

void galaxy_init(uint32_t seed);
void galaxy_draw(float yaw, float pitch);
