#include <libdragon.h>
#include <math.h>
#include "galaxy.h"
#include "rng.h"
#include "skyproj.h"

#define NUM_POINTS   170
#define CORE_POINTS   40        /* of NUM_POINTS, clustered in the bright core */
#define NUM_ARMS       2
#define SPIN          (2.4f * 6.2831853f)   /* total winding of each arm */

#define GALAXY_AZ    0.40f      /* fixed sky direction — forward and to the right */
#define GALAXY_EL    0.32f      /* and up */
#define PIX_RADIUS  72.0f       /* on-screen radius of the disc */
#define NUM_BUCKET     6        /* color buckets, drawn in grouped fill passes */

/* Color palette: 0 = warm core, 1..5 = arm inner→outer (blue-white → magenta). */
static const uint8_t PAL[NUM_BUCKET][3] = {
    {255, 226, 160},   /* 0 core warm      */
    {196, 212, 255},   /* 1 inner blue-white */
    {150, 184, 248},   /* 2                  */
    {148, 158, 224},   /* 3                  */
    {176, 128, 206},   /* 4 magenta          */
    {110,  82, 140},   /* 5 faint outer      */
};

/* Local disc coordinates (unit-ish) and color bucket per point. */
static float   lx[NUM_POINTS], ly[NUM_POINTS];
static uint8_t bucket[NUM_POINTS];

void galaxy_init(uint32_t seed) {
    Rng rng;
    rng_seed(&rng, seed);

    int idx = 0;

    /* Core: a dense warm cluster near the center (sqrt for a uniform disc). */
    for (int c = 0; c < CORE_POINTS; c++, idx++) {
        float rr = sqrtf(rng_unit(&rng)) * 0.16f;
        float aa = rng_unit(&rng) * 6.2831853f;
        lx[idx] = rr * cosf(aa);
        ly[idx] = rr * sinf(aa);
        bucket[idx] = 0;
    }

    /* Arms: points along logarithmic spirals, scattered to give the arms width. */
    int arm_pts = NUM_POINTS - CORE_POINTS;
    for (int k = 0; k < arm_pts; k++, idx++) {
        int   arm  = k % NUM_ARMS;
        float t    = (float)(k / NUM_ARMS) / (float)(arm_pts / NUM_ARMS); /* 0..1 */
        float base = arm * (6.2831853f / NUM_ARMS);
        float th   = base + t * SPIN;
        float r    = 0.12f + t * 0.88f;

        float sc = (rng_unit(&rng) - 0.5f) * 0.18f * (0.4f + t);
        lx[idx] = r * cosf(th) - sc * sinf(th) + (rng_unit(&rng) - 0.5f) * 0.05f;
        ly[idx] = r * sinf(th) + sc * cosf(th) + (rng_unit(&rng) - 0.5f) * 0.05f;

        /* Arm color bucket by radius along the arm: inner (1) → outer (5). */
        int bk = 1 + (int)(t * 4.999f);
        if (bk > 5) bk = 5;
        bucket[idx] = (uint8_t)bk;
    }
}

void galaxy_draw(float yaw, float pitch) {
    float cx, cy;
    sky_project(GALAXY_AZ, GALAXY_EL, yaw, pitch, &cx, &cy);

    /* Cull when the whole disc is off-screen (you're not looking at it). */
    if (cx < -PIX_RADIUS || cx > 320.0f + PIX_RADIUS ||
        cy < -PIX_RADIUS || cy > 240.0f + PIX_RADIUS)
        return;

    for (int bk = 0; bk < NUM_BUCKET; bk++) {
        rdpq_set_mode_fill(RGBA32(PAL[bk][0], PAL[bk][1], PAL[bk][2], 255));
        int sz = (bk == 0) ? 2 : 1;   /* core points slightly larger */
        for (int i = 0; i < NUM_POINTS; i++) {
            if (bucket[i] != bk) continue;
            int x = (int)(cx + lx[i] * PIX_RADIUS);
            int y = (int)(cy + ly[i] * PIX_RADIUS);
            if (x < 0 || x > 320 - sz - 1 || y < 0 || y > 240 - sz - 1) continue;
            rdpq_fill_rectangle(x, y, x + sz + 1, y + sz + 1);
        }
    }
}
