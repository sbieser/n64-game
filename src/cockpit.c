#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "cockpit.h"
#include "camera.h"

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

/* Indicator cluster — sits in the physical dashboard zone below the 3D frame */
#define IND_X0      32
#define IND_SQ       7
#define IND_GAP      3
#define IND_STEP    (IND_SQ + IND_GAP)
#define IND_Y1     216
#define IND_Y2     223
#define IND_COUNT    8

/* Signal screen (waveform — strength only, no direction) */
#define SCR_BOR_X1  250
#define SCR_BOR_Y1  204
#define SCR_BOR_X2  288
#define SCR_BOR_Y2  238
#define SCR_X1      252
#define SCR_Y1      206
#define SCR_X2      286
#define SCR_Y2      236

/* Bearing console — horizontal direction indicator, center-dashboard.
 * Only activates above BRG_THRESHOLD signal strength; dark/off below it. */
#define BRG_BOR_X1  134
#define BRG_BOR_Y1  213
#define BRG_BOR_X2  246
#define BRG_BOR_Y2  232
#define BRG_X1      136
#define BRG_Y1      215
#define BRG_X2      244
#define BRG_Y2      230
#define BRG_THRESHOLD  0.15f    /* signal must exceed this before bearing shows */

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
     * camera_basis() gives the same right/up axes t3d_viewport_look_at uses,
     * so the frame tracks the view exactly at any yaw/pitch.  The model's
     * local +Z faces the camera (−look) because the cockpit was modelled in
     * Blender with its opening toward the viewer.
     * T3DMat4 is column-major: m[col][row].
     */
    CameraBasis cb = camera_basis(lookX, lookY, lookZ);

    float cx = posX + lookX * COCKPIT_DIST;
    float cy = posY + lookY * COCKPIT_DIST;
    float cz = posZ + lookZ * COCKPIT_DIST;

    T3DMat4 mat;
    mat.m[0][0] =  cb.rightX; mat.m[0][1] = cb.rightY; mat.m[0][2] =  cb.rightZ; mat.m[0][3] = 0.0f;
    mat.m[1][0] =  cb.upX;    mat.m[1][1] = cb.upY;    mat.m[1][2] =  cb.upZ;    mat.m[1][3] = 0.0f;
    mat.m[2][0] = -lookX;     mat.m[2][1] = -lookY;    mat.m[2][2] = -lookZ;     mat.m[2][3] = 0.0f;
    mat.m[3][0] =  cx;        mat.m[3][1] = cy;        mat.m[3][2] =  cz;        mat.m[3][3] = 1.0f;

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
    /* No T3D_FLAG_DEPTH — cockpit must always render on top of world geometry.
     * Depth test would let close world objects (Pioneer, obstacles) bleed through
     * the frame. Drawing after the scene without testing is the correct approach
     * for any geometry that acts as a fixed HUD shell. */
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_CULL_BACK);
    t3d_matrix_push(&modelMats[frameIdx]);
    t3d_model_draw(model);
    t3d_matrix_pop(1);

    frameIdx = (frameIdx + 1) % 3;
}

static uint32_t hudFrame = 0;

void cockpit_draw_hud(float oxygen_level, float signal_h, float signal_strength) {
    hudFrame++;

    /* Oxygen indicator cluster */
    int lit = (int)ceilf(oxygen_level * (float)IND_COUNT);
    if (lit < 0)         lit = 0;
    if (lit > IND_COUNT) lit = IND_COUNT;
    bool critical = (oxygen_level <= 0.25f);
    for (int i = 0; i < IND_COUNT; i++) {
        int x1 = IND_X0 + i * IND_STEP;
        rdpq_set_mode_fill(i < lit ? (critical ? COL_RED : COL_CYAN) : COL_IND_OFF);
        rdpq_fill_rectangle(x1, IND_Y1, x1 + IND_SQ, IND_Y2);
    }

    /* Signal screen — waveform amplitude only, no direction.
     * The screen tells you "something is broadcasting" and how strongly,
     * but not where.  Direction is left entirely to the audio pan and
     * to the separate bearing console below. */
    rdpq_set_mode_fill(COL_FRAME);
    rdpq_fill_rectangle(SCR_BOR_X1, SCR_BOR_Y1, SCR_BOR_X2, SCR_BOR_Y2);
    rdpq_set_mode_fill(COL_SCR_BG);
    rdpq_fill_rectangle(SCR_X1, SCR_Y1, SCR_X2, SCR_Y2);

    #define WAVE_PTS   10
    #define WAVE_SPEED 0.05f
    int   scr_cy = (SCR_Y1 + SCR_Y2) / 2;
    float amp    = signal_strength * 5.0f;
    float t_base = (float)hudFrame * WAVE_SPEED;
    rdpq_set_mode_fill(COL_CYAN);
    for (int i = 0; i < WAVE_PTS; i++) {
        float t  = (float)i / (float)(WAVE_PTS - 1);
        int   px = SCR_X1 + 1 + (int)(t * (float)(SCR_X2 - SCR_X1 - 3));
        float ph = t * 6.28f - t_base;
        int   py = scr_cy + (int)(amp * sinf(ph));
        if (py < SCR_Y1 + 1) py = SCR_Y1 + 1;
        if (py > SCR_Y2 - 2) py = SCR_Y2 - 2;
        rdpq_fill_rectangle(px, py, px + 3, py + 2);
    }

    /* Bearing console — horizontal direction indicator.
     * Dark and off below threshold: you must get close enough for a signal
     * lock before the bearing instrument activates.  Above threshold a dim
     * track appears with a bright marker that slides left/right. */
    rdpq_set_mode_fill(COL_FRAME);
    rdpq_fill_rectangle(BRG_BOR_X1, BRG_BOR_Y1, BRG_BOR_X2, BRG_BOR_Y2);
    rdpq_set_mode_fill(COL_SCR_BG);
    rdpq_fill_rectangle(BRG_X1, BRG_Y1, BRG_X2, BRG_Y2);

    if (signal_strength >= BRG_THRESHOLD) {
        int track_w  = BRG_X2 - BRG_X1 - 2;   /* usable track pixels */
        int track_cy = (BRG_Y1 + BRG_Y2) / 2;

        /* Dim centerline and edge ticks */
        rdpq_set_mode_fill(COL_IND_OFF);
        rdpq_fill_rectangle(BRG_X1 + 1, track_cy, BRG_X2 - 1, track_cy + 1);
        int mid_x = BRG_X1 + 1 + track_w / 2;
        rdpq_fill_rectangle(mid_x - 1, BRG_Y1 + 2, mid_x + 1, BRG_Y2 - 2);

        /* Sliding marker: 4px wide × 8px tall, clamped inside the track */
        float t     = (signal_h + 1.0f) * 0.5f;   /* 0=left, 1=right */
        int   mk_x  = BRG_X1 + 1 + (int)(t * (float)(track_w - 4));
        if (mk_x < BRG_X1 + 1)      mk_x = BRG_X1 + 1;
        if (mk_x > BRG_X2 - 5)      mk_x = BRG_X2 - 5;
        rdpq_set_mode_fill(COL_CYAN);
        rdpq_fill_rectangle(mk_x, BRG_Y1 + 2, mk_x + 4, BRG_Y2 - 2);
    }
}
