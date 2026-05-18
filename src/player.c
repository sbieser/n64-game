/**
 * player.c — Player ship rendered from a loaded T3D model.
 *
 * Model: freighter.t3dm — "The Freighter", a compact industrial ship built in
 * Blender. Vertex colors carry all visual information; RDPQ_COMBINER_SHADE
 * (set in scene_rails) drives them through the RSP lighting pass.
 *
 * Scale 0.05 maps the Blender-unit model (≈3 units wide at base-scale 64) to
 * roughly 10 game units. Rotation π around Y flips the nose from -Z to +Z so
 * the ship faces forward along the rail.
 */

#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "player.h"

static T3DModel  *shipModel;
static T3DMat4FP *shipMat;

void player_init(void) {
    shipModel = t3d_model_load("rom:/freighter.t3dm");
    shipMat   = malloc_uncached(sizeof(T3DMat4FP));
}

void player_update(float lateralPos, float verticalPos, float railZ) {
    t3d_mat4fp_from_srt_euler(shipMat,
        (float[3]){0.05f, 0.05f, 0.05f},
        (float[3]){0.0f, (float)M_PI, 0.0f},
        (float[3]){lateralPos, 2.0f + verticalPos, railZ - 3.0f}
    );
}

void player_draw(void) {
    t3d_matrix_push(shipMat);
    t3d_model_draw(shipModel);
    t3d_matrix_pop(1);
}
