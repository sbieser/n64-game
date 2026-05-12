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

const Obstacle obstacles[NUM_OBSTACLES] = {
    {-570,   0,   0, 7.0f, SHAPE_CUBE,    0.3f, 1.0f, 0.0f},
    {-520,  14,  -4, 7.0f, SHAPE_TETRA,   0.4f, 0.7f, 1.0f},
    {-470,  -9,   7, 7.0f, SHAPE_OCTA,    0.2f, 1.2f, 2.1f},
    {-420,   0,  -6, 7.0f, SHAPE_PYRAMID, 0.5f, 0.4f, 0.5f},
    {-370,  18,   0, 7.0f, SHAPE_PRISM,   0.3f, 0.8f, 3.2f},
    {-320, -15,   5, 7.0f, SHAPE_CUBE,    0.6f, 0.5f, 1.7f},
    {-270,   0,   8, 7.0f, SHAPE_TETRA,   0.4f, 1.1f, 2.8f},
    {-220, -16,  -7, 7.0f, SHAPE_OCTA,    0.2f, 0.9f, 0.3f},
    {-170,  12,   0, 7.0f, SHAPE_PYRAMID, 0.5f, 0.6f, 1.4f},
    {-120,   0,   6, 7.0f, SHAPE_PRISM,   0.3f, 1.3f, 4.1f},
    { -70, -11,  -8, 7.0f, SHAPE_CUBE,    0.4f, 0.7f, 2.2f},
    { -20,   9,   4, 7.0f, SHAPE_OCTA,    0.6f, 0.4f, 3.7f},
};

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
