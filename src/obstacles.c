/**
 * obstacles.c — Obstacle table, matrices, collision, and drawing.
 *
 * obstacles_init()            — allocate model matrices
 * obstacles_update(rotAngle)  — rebuild matrices from current rotation angle
 * obstacles_check_collision() — return 1 if camera overlaps any obstacle
 * obstacles_draw()            — push/draw/pop each obstacle
 */

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "obstacles.h"

Obstacle obstacles[NUM_OBSTACLES];

/* Rail bounds — must match scene_rails.c */
#define OBS_RAIL_START  -600.0f
#define OBS_RAIL_END      50.0f
#define OBS_LAT_MAX       28.0f   /* slightly inside LATERAL_MAX so nothing hugs the wall */
#define OBS_VERT_MAX      14.0f   /* slightly inside VERTICAL_MAX */

static uint32_t rng_state;

static uint32_t rng_next(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

void obstacles_generate(uint32_t seed) {
    rng_state = seed;
    float rail_len = OBS_RAIL_END - OBS_RAIL_START - 60.0f; /* leave buffer at end */
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        obstacles[i].worldZ    = OBS_RAIL_START + (rng_next() % 10000) / 10000.0f * rail_len;
        obstacles[i].x         = ((int)(rng_next() % 1000) - 500) / 500.0f * OBS_LAT_MAX;
        obstacles[i].y         = ((int)(rng_next() % 1000) - 500) / 500.0f * OBS_VERT_MAX;
        obstacles[i].hitRadius = 7.0f;
        obstacles[i].shape     = (int)(rng_next() % 5);
        obstacles[i].rotSpeedX = 0.2f + (rng_next() % 100) / 250.0f;
        obstacles[i].rotSpeedY = 0.2f + (rng_next() % 100) / 250.0f;
        obstacles[i].rotPhase  = (rng_next() % 628) / 100.0f;
    }
}

/* One model matrix per obstacle in a contiguous uncached block.
 * Safe to use one per obstacle (not three) because rdpq_detach_show()
 * fully syncs the RSP/RDP pipeline before the next frame's CPU writes. */
static T3DMat4FP *obsMats;

void obstacles_init(void) {
    obsMats = malloc_uncached(sizeof(T3DMat4FP) * NUM_OBSTACLES);
}

#define DRAW_DIST 160.0f

void obstacles_update(float rotAngle, float camZ) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        const Obstacle *o = &obstacles[i];
        float dz = o->worldZ - camZ;
        if (dz < -20.0f || dz > DRAW_DIST) continue;
        float angle = rotAngle + o->rotPhase;
        t3d_mat4fp_from_srt_euler(&obsMats[i],
            (float[3]){1.0f, 1.0f, 1.0f},
            (float[3]){o->rotSpeedX * angle, o->rotSpeedY * angle, 0.0f},
            (float[3]){o->x, o->y, o->worldZ}
        );
    }
}

/* Returns 1 if the player figure overlaps any obstacle, 0 otherwise.
 * playerZ is the figure's world Z (railZ - 3). The Z window is symmetric
 * around the obstacle center — obstacles extend ±10 units, player ±2. */
int obstacles_check_collision(float playerZ, float lateralPos, float verticalPos) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        const Obstacle *o = &obstacles[i];
        float dz = playerZ - o->worldZ;
        if (dz > -10.0f && dz < 10.0f) {
            float dx = lateralPos - o->x;
            float dy = verticalPos - o->y;
            if (dx*dx + dy*dy < o->hitRadius * o->hitRadius)
                return 1;
        }
    }
    return 0;
}

void obstacles_draw(float camZ) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        const Obstacle *o = &obstacles[i];
        float dz = o->worldZ - camZ;
        if (dz < -20.0f || dz > DRAW_DIST) continue;
        t3d_matrix_push(&obsMats[i]);
        draw_shape(o->shape);
        t3d_matrix_pop(1);
    }
}
