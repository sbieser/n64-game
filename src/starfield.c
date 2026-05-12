/**
 * starfield.c — Static background star field.
 *
 * Stars are tiny quads (2 triangles each) scattered in world space across
 * the full rail Z range. The vertex buffer is filled once at init and never
 * touched again — stars are fixed world objects, not updated per frame.
 *
 * Rendered with an identity model matrix so vertex positions are treated
 * as world space directly. T3D_FLAG_NO_LIGHT is set by the caller before
 * starfield_draw() so stars glow at full vertex color regardless of lighting.
 *
 * Each quad faces -Z (toward the approaching camera). Back-face culling
 * automatically discards any star that ends up behind the camera.
 *
 * Batching: RSP vertex cache holds 70 slots. 4 verts per star → 17 stars
 * per batch (68 verts), leaving 2 slots of headroom.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "starfield.h"

#define NUM_STARS        150
#define STAR_HALF        1.5f   /* half-size of each star quad in world units */
#define STARS_PER_BATCH  17     /* 17 × 4 verts = 68, fits RSP cache of 70   */

static T3DVertPacked *starBuf;    /* 2 packed structs per star = NUM_STARS * 2 */
static T3DMat4FP     *identityMat;

/* Reproducible LCG — avoids stdlib rand() and gives the same star pattern
 * every boot, which is what we want for a fixed background. */
static unsigned int lcgNext(unsigned int *s) {
    *s = *s * 1664525u + 1013904223u;
    return *s >> 16;  /* top 16 bits have better randomness in LCGs */
}

void starfield_init(float railStart, float railEnd) {
    starBuf     = malloc_uncached(sizeof(T3DVertPacked) * NUM_STARS * 2);
    identityMat = malloc_uncached(sizeof(T3DMat4FP));

    /* Identity: scale 1, no rotation, no translation.
     * Pushing this matrix means vertex positions reach the RSP unchanged —
     * model space == world space. */
    t3d_mat4fp_from_srt_euler(identityMat,
        (float[3]){1,1,1}, (float[3]){0,0,0}, (float[3]){0,0,0});

    float railLen = railEnd - railStart;
    unsigned int seed = 0xDEADBEEFu;

    for (int i = 0; i < NUM_STARS; i++) {
        /* Scatter wide in X and Y to fill the peripheral space around the rail.
         * Y is offset +8 to center on the camera's height. */
        float x = (float)(lcgNext(&seed) % 161) - 80.0f;
        float y = (float)(lcgNext(&seed) % 141) - 70.0f + 8.0f;
        float z = railStart + (float)(lcgNext(&seed) % 10000) / 10000.0f * railLen;

        /* Mostly white, occasional blue-tinted or warm stars. */
        uint32_t color;
        switch (i % 5) {
            case 3:  color = 0xCCDDFFFF; break; /* blue-white */
            case 4:  color = 0xFFEECCFF; break; /* warm       */
            default: color = 0xFFFFFFFF; break; /* white      */
        }

        /* Quad vertices, all at the same Z (flat face):
         *   v0: bottom-left   v1: bottom-right
         *   v2: top-right     v3: top-left
         *
         * Winding (0,2,1) and (0,3,2) produces a normal in -Z, which is the
         * direction the camera approaches from. The face is front-facing to
         * the camera and back-face culling leaves it visible. */
        float S = STAR_HALF;
        int b = i * 2;
        starBuf[b] = (T3DVertPacked){
            .posA={(int16_t)(x-S), (int16_t)(y-S), (int16_t)z},
            .rgbaA=color,
            .posB={(int16_t)(x+S), (int16_t)(y-S), (int16_t)z},
            .rgbaB=color,
        };
        starBuf[b+1] = (T3DVertPacked){
            .posA={(int16_t)(x+S), (int16_t)(y+S), (int16_t)z},
            .rgbaA=color,
            .posB={(int16_t)(x-S), (int16_t)(y+S), (int16_t)z},
            .rgbaB=color,
        };
    }
}

void starfield_draw(void) {
    /* Push identity so star world-space positions pass through untransformed. */
    t3d_matrix_push(identityMat);

    for (int i = 0; i < NUM_STARS; i += STARS_PER_BATCH) {
        int batch = NUM_STARS - i;
        if (batch > STARS_PER_BATCH) batch = STARS_PER_BATCH;

        /* Load this batch's vertices starting at RSP cache slot 0. */
        t3d_vert_load(starBuf + i * 2, 0, batch * 4);

        for (int j = 0; j < batch; j++) {
            int b = j * 4;
            t3d_tri_draw(b+0, b+2, b+1); /* lower-right triangle */
            t3d_tri_draw(b+0, b+3, b+2); /* upper-left  triangle */
        }
        t3d_tri_sync();
    }

    t3d_matrix_pop(1);
}
