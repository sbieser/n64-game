#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "beacon.h"
#include "rng.h"

#define NUM_RINGS      3
#define SEGMENTS       28
#define R_MAX          80.0f     /* outer radius the ring grows to */
#define THICKNESS       1.5f     /* annulus half-width */
#define EXPAND_FRAMES  240       /* ~4s to expand fully — "very slow" */
#define GAP_MIN        120       /* idle frames between this ring's pulses */
#define GAP_MAX        360
#define BASE_BRIGHT    110.0f    /* peak channel brightness — dim by design */

static float center[3];
static Rng   rng;

/* Per ring: a frame timer. <0 = idle (counting up toward a new pulse);
 * 0..EXPAND_FRAMES = expanding. */
static int   timer[NUM_RINGS];

/* Separate vertex slice per ring so a slice is never overwritten while the RSP
 * may still be DMAing it within the same frame. */
static T3DVertPacked *verts;        /* NUM_RINGS * SEGMENTS packed (2 verts each) */
static T3DMat4FP     *identityMat;

static int new_gap(void) {
    return GAP_MIN + (int)rng_range(&rng, GAP_MAX - GAP_MIN);
}

void beacon_init(uint32_t seed, float cx, float cy, float cz) {
    center[0] = cx; center[1] = cy; center[2] = cz;
    rng_seed(&rng, seed);

    /* Stagger the rings with independent random gaps so they never sync. */
    for (int k = 0; k < NUM_RINGS; k++)
        timer[k] = -new_gap() - k * (GAP_MIN / NUM_RINGS);

    verts       = malloc_uncached(sizeof(T3DVertPacked) * NUM_RINGS * SEGMENTS);
    identityMat = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(identityMat,
        (float[3]){1, 1, 1}, (float[3]){0, 0, 0}, (float[3]){0, 0, 0});
}

void beacon_update(void) {
    for (int k = 0; k < NUM_RINGS; k++) {
        timer[k]++;
        if (timer[k] > EXPAND_FRAMES)
            timer[k] = -new_gap();   /* finished — wait a fresh random gap */
    }
}

/* Build one ring's annulus (flat, in the XZ plane) into its vertex slice. */
static void build_ring(int k, float r, uint32_t col) {
    float inner = r - THICKNESS;
    if (inner < 0.0f) inner = 0.0f;
    float outer = r + THICKNESS;
    T3DVertPacked *v = verts + k * SEGMENTS;

    for (int i = 0; i < SEGMENTS; i++) {
        float a  = (6.2831853f * i) / SEGMENTS;
        float ca = cosf(a), sa = sinf(a);
        v[i] = (T3DVertPacked){
            .posA = {(int16_t)(center[0] + inner * ca), (int16_t)center[1],
                     (int16_t)(center[2] + inner * sa)},
            .rgbaA = col,
            .posB = {(int16_t)(center[0] + outer * ca), (int16_t)center[1],
                     (int16_t)(center[2] + outer * sa)},
            .rgbaB = col,
        };
    }
}

void beacon_draw(void) {
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    /* No CULL_BACK — the flat ring is viewed from either side. */
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_NO_LIGHT);
    t3d_matrix_push(identityMat);

    for (int k = 0; k < NUM_RINGS; k++) {
        if (timer[k] < 0 || timer[k] > EXPAND_FRAMES) continue;

        float t = (float)timer[k] / (float)EXPAND_FRAMES;
        float r = t * R_MAX;
        float b = BASE_BRIGHT * (1.0f - t);    /* fade as it expands */

        /* Dim cold violet (more blue than red). */
        uint8_t R = (uint8_t)(b * 0.55f);
        uint8_t G = (uint8_t)(b * 0.45f);
        uint8_t B = (uint8_t)(b);
        uint32_t col = ((uint32_t)R << 24) | ((uint32_t)G << 16) | ((uint32_t)B << 8) | 0xFF;

        build_ring(k, r, col);

        /* Strip of 2·SEGMENTS verts: per segment, inner=2i, outer=2i+1. */
        t3d_vert_load(verts + k * SEGMENTS, 0, SEGMENTS * 2);
        for (int i = 0; i < SEGMENTS; i++) {
            int in0 = 2 * i,        out0 = 2 * i + 1;
            int in1 = 2 * ((i + 1) % SEGMENTS), out1 = in1 + 1;
            t3d_tri_draw(in0, out0, in1);
            t3d_tri_draw(out0, out1, in1);
        }
        t3d_tri_sync();
    }

    t3d_matrix_pop(1);
}
