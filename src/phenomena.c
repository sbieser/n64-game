#include "phenomena.h"

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

static uint32_t rng_state;

static uint32_t rng_next(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

void phenomena_generate(uint32_t seed) {
    /* Perturb the seed so this stream is independent from the obstacle stream */
    rng_state = seed ^ 0xFEED0001u;

    /* One field per rail segment — guarantees no overlap.
     * Each field occupies at most 65% of its segment, starting in the first
     * 35% of that segment, so z_end never reaches the next segment's z_start. */
    float rail_len = RAIL_END - RAIL_START - 100.0f;
    float seg      = rail_len / NUM_FIELDS;

    for (int i = 0; i < NUM_FIELDS; i++) {
        float seg_start = RAIL_START + i * seg;
        float max_len   = seg * 0.65f;
        float z   = seg_start + (rng_next() % 10000) / 10000.0f * (seg - max_len);
        float len = seg * 0.35f + (rng_next() % 10000) / 10000.0f * (max_len - seg * 0.35f);
        float mag = 0.05f + (float)(rng_next() % 100) / 1000.0f;
        float dir = (rng_next() % 2) ? 1.0f : -1.0f;
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
