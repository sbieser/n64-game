/*
 * phenomena.c — gravitational fields along the rail.
 *
 * Each rail segment gets exactly one field, placed so it never spills into the
 * neighbouring segment (it occupies at most 65% of its segment, starting within
 * the first 35%).  That guarantees fields never overlap, so phenomena_update
 * can stop at the first match.
 *
 * The field layout is seed-derived but on a SEPARATE stream from the obstacle
 * field: the seed is XORed with a domain constant before seeding the generator,
 * so gravity fields and obstacles are uncorrelated even though both come from
 * the same run seed.  See rng.h for the determinism rationale.
 *
 * The public API and lifecycle are documented in phenomena.h.
 */

#include "phenomena.h"
#include "rng.h"

/* Rail bounds — must match scene_rails.c */
#define RAIL_START  -600.0f
#define RAIL_END      50.0f
#define NUM_FIELDS     3

typedef struct {
    float z_start, z_end;
    float pull;   /* lateral units/frame — negative = left, positive = right */
} GravField;

static GravField fields[NUM_FIELDS];
static float     current_pull = 0.0f;

void phenomena_generate(uint32_t seed) {
    /* Perturb the seed so this stream is independent from the obstacle stream.
     * Same seed, different domain constant → a parallel but uncorrelated run
     * of draws, so gravity fields don't track obstacle positions. */
    Rng rng;
    rng_seed(&rng, seed ^ 0xFEED0001u);

    /* One field per rail segment — guarantees no overlap.
     * Each field occupies at most 65% of its segment, starting in the first
     * 35% of that segment, so z_end never reaches the next segment's z_start. */
    float rail_len = RAIL_END - RAIL_START - 100.0f;
    float seg      = rail_len / NUM_FIELDS;

    for (int i = 0; i < NUM_FIELDS; i++) {
        float seg_start = RAIL_START + i * seg;
        float max_len   = seg * 0.65f;
        float z   = seg_start + rng_unit(&rng) * (seg - max_len);
        float len = seg * 0.35f + rng_unit(&rng) * (max_len - seg * 0.35f);
        float mag = 0.05f + (float)(rng_next(&rng) % 100) / 1000.0f;
        float dir = rng_range(&rng, 2) ? 1.0f : -1.0f;
        fields[i].z_start = z;
        fields[i].z_end   = z + len;
        fields[i].pull    = mag * dir;
    }
}

void phenomena_update(float railZ, float *lateralPos) {
    current_pull = 0.0f;
    for (int i = 0; i < NUM_FIELDS; i++) {
        if (railZ >= fields[i].z_start && railZ <= fields[i].z_end) {
            current_pull   = fields[i].pull;
            *lateralPos   += current_pull;
            break;
        }
    }
}

float phenomena_pull(void) { return current_pull; }

int phenomena_field_count(void) { return NUM_FIELDS; }

void phenomena_get_field(int i, float *z_start, float *z_end, float *pull) {
    *z_start = fields[i].z_start;
    *z_end   = fields[i].z_end;
    *pull    = fields[i].pull;
}
