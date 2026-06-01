#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "dust.h"
#include "rng.h"
#include "camera.h"

#define NUM_DUST    28
#define BOX         45.0f    /* half-extent of the dust volume around the camera */
#define DUST_HALF    1.0f    /* half-size of each billboard quad (world units).
                              * Must be ≥ ~1: vertex positions are int16 world
                              * units, so a sub-unit quad would round to zero area. */
#define QUADS_PER_BATCH 17   /* 17 × 4 = 68 verts, fits the 70-slot RSP cache */

#define DUST_COLOR  0x6A7080FF   /* dim blue-grey — subtle, not a starfield */

/* Camera-relative offsets and per-particle intrinsic drift. */
static float ox[NUM_DUST], oy[NUM_DUST], oz[NUM_DUST];
static float dx[NUM_DUST], dy[NUM_DUST], dz[NUM_DUST];

static T3DVertPacked *verts;       /* 2 packed structs (4 verts) per particle */
static T3DMat4FP     *identityMat; /* particles are built in world space */

void dust_init(uint32_t seed) {
    Rng rng;
    rng_seed(&rng, seed);
    for (int i = 0; i < NUM_DUST; i++) {
        ox[i] = (float)((int)rng_range(&rng, 2001) - 1000) / 1000.0f * BOX;
        oy[i] = (float)((int)rng_range(&rng, 2001) - 1000) / 1000.0f * BOX;
        oz[i] = (float)((int)rng_range(&rng, 2001) - 1000) / 1000.0f * BOX;
        /* Slow intrinsic drift: ±0.03 units/frame per axis. */
        dx[i] = (float)((int)rng_range(&rng, 201) - 100) / 100.0f * 0.03f;
        dy[i] = (float)((int)rng_range(&rng, 201) - 100) / 100.0f * 0.03f;
        dz[i] = (float)((int)rng_range(&rng, 201) - 100) / 100.0f * 0.03f;
    }

    verts       = malloc_uncached(sizeof(T3DVertPacked) * NUM_DUST * 2);
    identityMat = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(identityMat,
        (float[3]){1, 1, 1}, (float[3]){0, 0, 0}, (float[3]){0, 0, 0});
}

/* Wrap v into [-half, half) so particles recycle through the box. */
static float wrapf(float v, float half) {
    float period = 2.0f * half;
    v = fmodf(v + half, period);
    if (v < 0.0f) v += period;
    return v - half;
}

void dust_update(float camVelX, float camVelY, float camVelZ) {
    for (int i = 0; i < NUM_DUST; i++) {
        /* Camera moved by +vel, so the relative offset shifts by −vel; the
         * intrinsic drift is added on top. */
        ox[i] = wrapf(ox[i] + dx[i] - camVelX, BOX);
        oy[i] = wrapf(oy[i] + dy[i] - camVelY, BOX);
        oz[i] = wrapf(oz[i] + dz[i] - camVelZ, BOX);
    }
}

void dust_draw(float camX, float camY, float camZ,
               float lookX, float lookY, float lookZ) {
    /* Camera right/up to orient each quad to face the viewer. */
    CameraBasis cb = camera_basis(lookX, lookY, lookZ);
    float rx = cb.rightX * DUST_HALF, ry = cb.rightY * DUST_HALF, rz = cb.rightZ * DUST_HALF;
    float ux = cb.upX    * DUST_HALF, uy = cb.upY    * DUST_HALF, uz = cb.upZ    * DUST_HALF;

    for (int i = 0; i < NUM_DUST; i++) {
        float wx = camX + ox[i], wy = camY + oy[i], wz = camZ + oz[i];
        /* Four corners: BL, BR, TR, TL (same winding as the rails starfield). */
        int b = i * 2;
        verts[b] = (T3DVertPacked){
            .posA = {(int16_t)(wx - rx - ux), (int16_t)(wy - ry - uy), (int16_t)(wz - rz - uz)},
            .rgbaA = DUST_COLOR,
            .posB = {(int16_t)(wx + rx - ux), (int16_t)(wy + ry - uy), (int16_t)(wz + rz - uz)},
            .rgbaB = DUST_COLOR,
        };
        verts[b + 1] = (T3DVertPacked){
            .posA = {(int16_t)(wx + rx + ux), (int16_t)(wy + ry + uy), (int16_t)(wz + rz + uz)},
            .rgbaA = DUST_COLOR,
            .posB = {(int16_t)(wx - rx + ux), (int16_t)(wy - ry + uy), (int16_t)(wz - rz + uz)},
            .rgbaB = DUST_COLOR,
        };
    }

    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_NO_LIGHT);
    t3d_matrix_push(identityMat);

    for (int i = 0; i < NUM_DUST; i += QUADS_PER_BATCH) {
        int batch = NUM_DUST - i;
        if (batch > QUADS_PER_BATCH) batch = QUADS_PER_BATCH;
        t3d_vert_load(verts + i * 2, 0, batch * 4);
        for (int j = 0; j < batch; j++) {
            int v = j * 4;
            t3d_tri_draw(v + 0, v + 2, v + 1);
            t3d_tri_draw(v + 0, v + 3, v + 2);
        }
        t3d_tri_sync();
    }

    t3d_matrix_pop(1);
}
