/**
 * player.c — The player figure: two square pyramids sharing a waist vertex.
 *
 * The body is a normal upright pyramid (wide at feet, apex pointing up).
 * The head is an inverted pyramid (apex pointing down into the waist, wide
 * at the top). Together they form a diamond hourglass silhouette — abstract
 * but readable as a human figure at N64 resolution.
 *
 * Vertex layout (9 verts, 5 packed structs):
 *
 *   [5]--[6]       head base (y=+5), wide part at top
 *    |  / |
 *   [8]  [7]
 *     \ /
 *     [0]          shared waist / apex (y=+2)
 *     / \
 *   [4] [2]
 *    |   |
 *   [3]--[1]       body base (y=-3), wide part at bottom
 *      [1] = front-left of base (index offset — see below)
 *
 *   Body:  apex=v0  base corners=v1,v2,v3,v4
 *   Head:  apex=v0  base corners=v5,v6,v7,v8
 *
 * The figure is positioned 12 units ahead of the camera and 6 units below
 * camera height so it sits in the lower-center of the screen.
 */

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "player.h"

static T3DVertPacked *playerVerts;  /* 5 packed structs = 9 verts (last slot B unused) */
static T3DMat4FP     *playerMat;

static uint16_t packNorm(float x, float y, float z) {
    T3DVec3 v = {{x, y, z}};
    t3d_vec3_norm(&v);
    return t3d_vert_pack_normal(&v);
}

static void initPlayerVerts(void) {
    /* v0: shared waist — body apex (pointing up) and head apex (pointing down) */
    /* v1–v4: body base corners at y=-3, ±3 in X and Z */
    /* v5–v8: head base corners at y=+5, ±2 in X and Z */

    /* Pack 0: v0(waist), v1(body front-left) */
    playerVerts[0] = (T3DVertPacked){
        .posA={ 0,  2,  0}, .normA=packNorm( 0, 1, 0),   .rgbaA=0xAADDFFFF,
        .posB={-3, -3, -3}, .normB=packNorm(-1,-1,-1),   .rgbaB=0x4488CCFF,
    };
    /* Pack 1: v2(body front-right), v3(body back-right) */
    playerVerts[1] = (T3DVertPacked){
        .posA={ 3, -3, -3}, .normA=packNorm( 1,-1,-1),   .rgbaA=0x4488CCFF,
        .posB={ 3, -3,  3}, .normB=packNorm( 1,-1, 1),   .rgbaB=0x4488CCFF,
    };
    /* Pack 2: v4(body back-left), v5(head front-left) */
    playerVerts[2] = (T3DVertPacked){
        .posA={-3, -3,  3}, .normA=packNorm(-1,-1, 1),   .rgbaA=0x4488CCFF,
        .posB={-2,  5, -2}, .normB=packNorm(-1, 1,-1),   .rgbaB=0xFFEEDDFF,
    };
    /* Pack 3: v6(head front-right), v7(head back-right) */
    playerVerts[3] = (T3DVertPacked){
        .posA={ 2,  5, -2}, .normA=packNorm( 1, 1,-1),   .rgbaA=0xFFEEDDFF,
        .posB={ 2,  5,  2}, .normB=packNorm( 1, 1, 1),   .rgbaB=0xFFEEDDFF,
    };
    /* Pack 4: v8(head back-left), slot B unused */
    playerVerts[4] = (T3DVertPacked){
        .posA={-2,  5,  2}, .normA=packNorm(-1, 1, 1),   .rgbaA=0xFFEEDDFF,
    };
}

void player_init(void) {
    playerVerts = malloc_uncached(sizeof(T3DVertPacked) * 5);
    playerMat   = malloc_uncached(sizeof(T3DMat4FP));
    initPlayerVerts();
}

void player_update(float lateralPos, float verticalPos, float railZ) {
    /* Position the figure 12 units ahead of the camera (camZ = railZ - 15,
     * figure at railZ - 3) and 6 units below camera height (cam y = 8 + vp,
     * figure y = 2 + vp). This places it in the lower-centre of the view. */
    t3d_mat4fp_from_srt_euler(playerMat,
        (float[3]){1.0f, 1.0f, 1.0f},
        (float[3]){0.0f, 0.0f, 0.0f},
        (float[3]){lateralPos, 2.0f + verticalPos, railZ - 3.0f}
    );
}

void player_draw(void) {
    t3d_matrix_push(playerMat);
    t3d_vert_load(playerVerts, 0, 9);

    /* Body — upright pyramid, apex=v0, base=v1..v4 */
    t3d_tri_draw(1, 2, 3); t3d_tri_draw(1, 3, 4); /* base  (normal -Y) */
    t3d_tri_draw(0, 2, 1);                          /* front (normal -Z) */
    t3d_tri_draw(0, 3, 2);                          /* right (normal +X) */
    t3d_tri_draw(0, 4, 3);                          /* back  (normal +Z) */
    t3d_tri_draw(0, 1, 4);                          /* left  (normal -X) */

    /* Head — inverted pyramid, apex=v0, base=v5..v8 at top */
    t3d_tri_draw(5, 7, 6); t3d_tri_draw(5, 8, 7); /* top   (normal +Y) */
    t3d_tri_draw(0, 5, 6);                          /* front             */
    t3d_tri_draw(0, 6, 7);                          /* right             */
    t3d_tri_draw(0, 7, 8);                          /* back              */
    t3d_tri_draw(0, 8, 5);                          /* left              */

    t3d_tri_sync();
    t3d_matrix_pop(1);
}
