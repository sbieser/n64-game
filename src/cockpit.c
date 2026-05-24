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
 *   Panel depth      : 17.5 units (thin bezel)
 *   Camera offset    : 150 units — the model face sits this far in front of
 *                      the player so it fills the view at the same angles as
 *                      the Blender mockup (FOV 65°, 320×240).
 *
 * No scale is applied in the model matrix; the model is already in world units.
 * Rotation matches the player's yaw (around world Y) then pitch (around local X).
 */

#define COCKPIT_DIST  150.0f   /* units in front of camera */

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
    /* Place the cockpit face COCKPIT_DIST units in front of the camera. */
    float cx = posX + lookX * COCKPIT_DIST;
    float cy = posY + lookY * COCKPIT_DIST;
    float cz = posZ + lookZ * COCKPIT_DIST;

    /*
     * Derive yaw and pitch from the look vector so the cockpit rotates with
     * the camera.  lookX/Y/Z are already (sin(yaw)*cos(pitch), sin(pitch),
     * -cos(yaw)*cos(pitch)), so:
     */
    float pitch = asinf(lookY);
    float yaw   = atan2f(lookX, -lookZ);

    float scale[3] = {1.0f, 1.0f, 1.0f};
    float rot[3]   = {pitch, yaw, 0.0f};
    float pos[3]   = {cx, cy, cz};
    t3d_mat4fp_from_srt_euler(&modelMats[frameIdx], scale, rot, pos);

    /* Cold blue-white directional from above — matches Blender mockup lighting */
    uint8_t amb[4] = {25, 28, 35, 255};
    uint8_t dir[4] = {130, 138, 158, 255};
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

void cockpit_draw_hud(float oxygen_level) {
    int lit = (int)ceilf(oxygen_level * (float)IND_COUNT);
    if (lit < 0)         lit = 0;
    if (lit > IND_COUNT) lit = IND_COUNT;
    bool critical = (oxygen_level <= 0.25f);

    for (int i = 0; i < IND_COUNT; i++) {
        int x1 = IND_X0 + i * IND_STEP;
        rdpq_set_mode_fill(i < lit ? (critical ? COL_RED : COL_CYAN) : COL_IND_OFF);
        rdpq_fill_rectangle(x1, IND_Y1, x1 + IND_SQ, IND_Y2);
    }

    /* Signal screen */
    rdpq_set_mode_fill(COL_FRAME);
    rdpq_fill_rectangle(SCR_BOR_X1, SCR_BOR_Y1, SCR_BOR_X2, SCR_BOR_Y2);
    rdpq_set_mode_fill(COL_SCR_BG);
    rdpq_fill_rectangle(SCR_X1, SCR_Y1, SCR_X2, SCR_Y2);
    rdpq_set_mode_fill(COL_CYAN);
    rdpq_fill_rectangle(DOT_X1, DOT_Y1, DOT_X2, DOT_Y2);
}
