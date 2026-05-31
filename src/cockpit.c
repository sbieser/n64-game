#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "cockpit.h"

/*
 * The cockpit frame model was built in Blender at 50× game-unit scale so
 * int16 vertex coords have sub-percent rounding error.  Key dimensions:
 *
 *   Strut inner edge : ±82 units from center
 *   Dashboard top    : -49 units below center
 *   Top aperture top :  +67 units above center
 *   Outer frame      : ±180 wide, ±150 tall — well past screen edges at any angle
 *   Panel depth      : 17.5 units (thin bezel)
 *   Camera offset    : 130 units — closer than the original 150, opening up the
 *                      viewport so more of the scene is visible.  fovY=65° at
 *                      320×240 means screen top = tan(32.5°)*130 = 82.7 units;
 *                      the outer frame at ±150 covers this with comfortable margin.
 *
 * No scale is applied in the model matrix; the model is already in world units.
 * Rotation is built directly from camera basis vectors (no Euler decomposition).
 */

#define COCKPIT_DIST  100.0f   /* units in front of camera */

/* Indicator cluster */
#define IND_X0      32
#define IND_SQ       7
#define IND_GAP      3
#define IND_STEP    (IND_SQ + IND_GAP)
#define IND_Y1     196
#define IND_Y2     203
#define IND_COUNT    8

/* Signal screen */
#define SCR_BOR_X1  250
#define SCR_BOR_Y1  184
#define SCR_BOR_X2  288
#define SCR_BOR_Y2  212
#define SCR_X1      252
#define SCR_Y1      186
#define SCR_X2      286
#define SCR_Y2      210
#define DOT_X1      265
#define DOT_Y1      194
#define DOT_X2      273
#define DOT_Y2      202

#define COL_CYAN    RGBA32(  0, 179, 217, 255)
#define COL_RED     RGBA32(217,  13,  13, 255)
#define COL_IND_OFF RGBA32(  5,   3,   2, 255)
#define COL_FRAME   RGBA32( 10,   6,   3, 255)
#define COL_SCR_BG  RGBA32(  4,   8,  17, 255)

static T3DModel  *model     = NULL;
static T3DMat4FP *modelMats = NULL;   /* triple-buffered */
static int        frameIdx  = 0;

void cockpit_init(void) {
    model     = t3d_model_load("rom:/cockpit.t3dm");
    modelMats = malloc_uncached(sizeof(T3DMat4FP) * 3);
}

void cockpit_draw_frame(float posX, float posY, float posZ,
                        float lookX, float lookY, float lookZ) {
    /*
     * Build the model matrix directly from the camera's look direction.
     * The same cross-product construction as t3d_viewport_look_at internally:
     *   side  = normalize(look × worldUp) = normalize((-lookZ, 0, lookX))
     *   camUp = side × look
     *
     * Pitch is clamped to ±80° by all scenes so sideLen = cos(pitch) > 0.17,
     * safe from the singularity where look is parallel to worldUp.
     *
     * The model's local +Z faces toward the camera (−look direction) because
     * the cockpit was built in Blender with its opening toward the camera.
     * T3DMat4 is column-major: m[col][row].
     */
    float sideLen = sqrtf(lookZ * lookZ + lookX * lookX);
    if (sideLen < 0.0001f) sideLen = 0.0001f;   /* guard against zero look vector */
    float sx = -lookZ / sideLen, sz =  lookX / sideLen;   /* sy = 0 always */
    /* camUp = side × look  (sy=0 simplifies the cross product) */
    float ux = -sz * lookY;
    float uy =  sz * lookX - sx * lookZ;   /* = sideLen */
    float uz =  sx * lookY;

    float cx = posX + lookX * COCKPIT_DIST;
    float cy = posY + lookY * COCKPIT_DIST;
    float cz = posZ + lookZ * COCKPIT_DIST;

    T3DMat4 mat;
    mat.m[0][0] =  sx;    mat.m[0][1] = 0.0f; mat.m[0][2] =  sz;    mat.m[0][3] = 0.0f;
    mat.m[1][0] =  ux;    mat.m[1][1] = uy;   mat.m[1][2] =  uz;    mat.m[1][3] = 0.0f;
    mat.m[2][0] = -lookX; mat.m[2][1] = -lookY; mat.m[2][2] = -lookZ; mat.m[2][3] = 0.0f;
    mat.m[3][0] =  cx;    mat.m[3][1] = cy;   mat.m[3][2] =  cz;    mat.m[3][3] = 1.0f;

    t3d_mat4_to_fixed_3x4(&modelMats[frameIdx], &mat);

    /* High ambient keeps the cockpit readable against dark space; directional
     * adds shape. Total target ~200 so the frame reads as mid-gray in the void. */
    uint8_t amb[4] = {85, 92, 110, 255};
    uint8_t dir[4] = {100, 108, 128, 255};
    T3DVec3 lightDir = {{0.3f, 1.0f, 0.2f}};
    t3d_vec3_norm(&lightDir);
    t3d_light_set_ambient(amb);
    t3d_light_set_directional(0, dir, &lightDir);
    t3d_light_set_count(1);

    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
    t3d_matrix_push(&modelMats[frameIdx]);
    t3d_model_draw(model);
    t3d_matrix_pop(1);

    frameIdx = (frameIdx + 1) % 3;
}

void cockpit_draw_hud(float oxygen_level,
                      float signal_h, float signal_v, float signal_strength) {
    int lit = (int)ceilf(oxygen_level * (float)IND_COUNT);
    if (lit < 0)         lit = 0;
    if (lit > IND_COUNT) lit = IND_COUNT;
    bool critical = (oxygen_level <= 0.25f);

    for (int i = 0; i < IND_COUNT; i++) {
        int x1 = IND_X0 + i * IND_STEP;
        rdpq_set_mode_fill(i < lit ? (critical ? COL_RED : COL_CYAN) : COL_IND_OFF);
        rdpq_fill_rectangle(x1, IND_Y1, x1 + IND_SQ, IND_Y2);
    }

    /* Signal screen — dot position encodes direction, size encodes strength */
    rdpq_set_mode_fill(COL_FRAME);
    rdpq_fill_rectangle(SCR_BOR_X1, SCR_BOR_Y1, SCR_BOR_X2, SCR_BOR_Y2);
    rdpq_set_mode_fill(COL_SCR_BG);
    rdpq_fill_rectangle(SCR_X1, SCR_Y1, SCR_X2, SCR_Y2);

    int scr_cx = (SCR_X1 + SCR_X2) / 2;   /* 269 */
    int scr_cy = (SCR_Y1 + SCR_Y2) / 2;   /* 198 */
    int dot_r  = 1 + (int)(signal_strength * 3.0f);   /* radius 1..4 */
    int dot_cx = scr_cx + (int)(signal_h *  10.0f);
    int dot_cy = scr_cy - (int)(signal_v *   7.0f);   /* Y flipped: up = smaller Y */
    if (dot_cx < SCR_X1 + dot_r) dot_cx = SCR_X1 + dot_r;
    if (dot_cx > SCR_X2 - dot_r) dot_cx = SCR_X2 - dot_r;
    if (dot_cy < SCR_Y1 + dot_r) dot_cy = SCR_Y1 + dot_r;
    if (dot_cy > SCR_Y2 - dot_r) dot_cy = SCR_Y2 - dot_r;
    rdpq_set_mode_fill(COL_CYAN);
    rdpq_fill_rectangle(dot_cx - dot_r, dot_cy - dot_r,
                        dot_cx + dot_r, dot_cy + dot_r);
}
