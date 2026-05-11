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
    {-220,  0,  0, 9.0f, SHAPE_CUBE,    0.3f, 1.0f, 0.0f},
    {-195,  8, -2, 9.0f, SHAPE_TETRA,   0.4f, 0.7f, 1.0f},
    {-170, -6,  3, 9.0f, SHAPE_OCTA,    0.2f, 1.2f, 2.1f},
    {-148,  0, -2, 9.0f, SHAPE_PYRAMID, 0.5f, 0.4f, 0.5f},
    {-125, 10,  0, 9.0f, SHAPE_PRISM,   0.3f, 0.8f, 3.2f},
    { -98, -8,  2, 9.0f, SHAPE_CUBE,    0.6f, 0.5f, 1.7f},
    { -74,  0,  4, 9.0f, SHAPE_TETRA,   0.4f, 1.1f, 2.8f},
    { -50,-10, -3, 9.0f, SHAPE_OCTA,    0.2f, 0.9f, 0.3f},
    { -28,  7,  0, 9.0f, SHAPE_PYRAMID, 0.5f, 0.6f, 1.4f},
    {  -8,  0,  0, 9.0f, SHAPE_PRISM,   0.3f, 1.3f, 4.1f},
    {  12, -8,  3, 9.0f, SHAPE_CUBE,    0.4f, 0.7f, 2.2f},
    {  30,  5, -4, 9.0f, SHAPE_OCTA,    0.6f, 0.4f, 3.7f},
};

/* One model matrix per obstacle in a contiguous uncached block.
 * Safe to use one per obstacle (not three) because rdpq_detach_show()
 * fully syncs the RSP/RDP pipeline before the next frame's CPU writes. */
static T3DMat4FP *obsMats;

void obstacles_init(void) {
    obsMats = malloc_uncached(sizeof(T3DMat4FP) * NUM_OBSTACLES);
}

void obstacles_update(float rotAngle) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        const Obstacle *o = &obstacles[i];
        float angle = rotAngle + o->rotPhase;
        t3d_mat4fp_from_srt_euler(&obsMats[i],
            (float[3]){1.0f, 1.0f, 1.0f},
            (float[3]){o->rotSpeedX * angle, o->rotSpeedY * angle, 0.0f},
            (float[3]){o->x, o->y, o->worldZ}
        );
    }
}

/* Returns 1 if the camera overlaps any obstacle, 0 otherwise.
 * Uses a forward-weighted Z window (-12 to +2) so the hit registers
 * while the shape is visually in front, not after it has flown past. */
int obstacles_check_collision(float camZ, float lateralPos, float verticalPos) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        const Obstacle *o = &obstacles[i];
        float dz = camZ - o->worldZ;
        if (dz > -12.0f && dz < 2.0f) {
            float dx = lateralPos - o->x;
            float dy = verticalPos - o->y;
            if (dx*dx + dy*dy < o->hitRadius * o->hitRadius)
                return 1;
        }
    }
    return 0;
}

void obstacles_draw(void) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        t3d_matrix_push(&obsMats[i]);
        draw_shape(obstacles[i].shape);
        t3d_matrix_pop(1);
    }
}
