#pragma once
#include <stdint.h>

/*
 * phenomena.h — gravitational fields along the rail.
 *
 * Invisible zones that pull the player laterally; they must be fought with the
 * stick to hold course.  Their positions and strengths are seed-derived, on a
 * stream kept independent from the obstacle field (the generator XORs the seed
 * with a domain constant) so fields and obstacles don't correlate.
 *
 * The query functions (pull / field_count / get_field) exist so a debug or
 * visualizer scene can render the otherwise-invisible fields.
 *
 * LIFECYCLE
 *   phenomena_generate(seed);              // on stage entry — place the fields
 *   phenomena_update(railZ, &lateralPos);  // each frame — apply pull in-place
 */

/* Place the gravity fields deterministically from the run seed. */
void  phenomena_generate(uint32_t seed);

/* If railZ is inside a field, add its lateral pull to *lateralPos (mutated
 * in place). Records the active pull for phenomena_pull(). */
void  phenomena_update(float railZ, float *lateralPos);

/* The lateral pull applied on the most recent phenomena_update (0 if none). */
float phenomena_pull(void);

/* --- Introspection (for debug / visualizer scenes) --- */

/* Number of gravity fields in the current layout. */
int   phenomena_field_count(void);

/* Read field i's Z extent and pull strength out through the pointers. */
void  phenomena_get_field(int i, float *z_start, float *z_end, float *pull);
