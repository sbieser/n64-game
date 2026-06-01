#include <libdragon.h>
#include <math.h>
#include "nebula.h"
#include "skyproj.h"

#define NUM_WASH   3
#define SEGMENTS   18          /* rim points of each soft blob */

/* Each wash: a fixed sky direction, a center color, a radius (px) and a peak
 * alpha. Colors are a touch brighter than they read, because additive blending
 * over the near-black void only adds light. */
static const float   WASH_AZ[NUM_WASH]    = { 0.10f, 3.14159f, -1.45f };
static const float   WASH_EL[NUM_WASH]    = { 0.05f, 0.00f,     0.12f };
static const uint8_t WASH_COL[NUM_WASH][3] = {
    {88, 52, 140},   /* violet — toward the signal/Pioneer */
    {34, 50, 110},   /* deep blue — behind                 */
    {64, 50, 130},   /* indigo — to one side               */
};
static const float WASH_RAD[NUM_WASH]   = { 165.0f, 150.0f, 140.0f };
static const float WASH_ALPHA[NUM_WASH] = { 0.55f,  0.45f,  0.50f  };

void nebula_draw(float yaw, float pitch) {
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    rdpq_mode_blender(RDPQ_BLENDER_ADDITIVE);

    rdpq_trifmt_t fmt = { .pos_offset = 0, .shade_offset = 2,
                          .tex_offset = -1, .z_offset = -1 };

    for (int w = 0; w < NUM_WASH; w++) {
        float cx, cy;
        sky_project(WASH_AZ[w], WASH_EL[w], yaw, pitch, &cx, &cy);

        float rad = WASH_RAD[w];
        if (cx < -rad || cx > 320.0f + rad || cy < -rad || cy > 240.0f + rad)
            continue;   /* fully off-screen — behind / out of view */

        float r = WASH_COL[w][0] / 255.0f;
        float g = WASH_COL[w][1] / 255.0f;
        float b = WASH_COL[w][2] / 255.0f;
        float a = WASH_ALPHA[w];

        float ctr[6] = { cx, cy, r, g, b, a };
        for (int i = 0; i < SEGMENTS; i++) {
            float a0 = (6.2831853f * i) / SEGMENTS;
            float a1 = (6.2831853f * (i + 1)) / SEGMENTS;
            float v0[6] = { cx + cosf(a0) * rad, cy + sinf(a0) * rad, r, g, b, 0.0f };
            float v1[6] = { cx + cosf(a1) * rad, cy + sinf(a1) * rad, r, g, b, 0.0f };
            rdpq_triangle(&fmt, ctr, v0, v1);
        }
    }

    rdpq_mode_blender(0);   /* disable additive so later layers aren't affected */
}
