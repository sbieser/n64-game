#include <libdragon.h>
#include <math.h>
#include "cockpit.h"

/*
 * All pixel coordinates derived from the Blender frustum mockup:
 *   FOV 65°, 320×240, camera at origin looking down +Y.
 *   Frustum half-width HW=0.6371, half-height HH=0.4778 at Y=1.
 *
 * Conversion: screen_px = (frust_x + HW) / (2*HW) * 320
 *             screen_py = (HH - frust_z) / (2*HH) * 240
 */

/* Frame geometry */
#define PIL_L_X1     0
#define PIL_L_X2    18
#define PIL_R_X1   302
#define PIL_R_X2   320
#define INNER_X1    18
#define INNER_X2   302
#define BAR_TOP_Y1   0
#define BAR_TOP_Y2   8
#define MID_Y1     150
#define MID_Y2     156
#define DASH_Y1    156
#define DASH_Y2    240

/* Indicator cluster — 8 squares, 7×7 px, 3px gap, starting at x=32, y=196 */
#define IND_X0      32
#define IND_SQ       7
#define IND_GAP      3
#define IND_STEP    (IND_SQ + IND_GAP)   /* 10 px per square */
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
#define DOT_X1      265   /* centered dot — will be driven by signal in game */
#define DOT_Y1      194
#define DOT_X2      273
#define DOT_Y2      202

/* Gloved hand */
#define SLEEVE_X1   244
#define SLEEVE_Y1   205
#define SLEEVE_X2   298
#define SLEEVE_Y2   240
#define CUFF_Y1     199
#define CUFF_Y2     205
#define THUMB_X1    233
#define THUMB_Y1    207
#define THUMB_X2    247
#define THUMB_Y2    221

/* Colors */
#define COL_FRAME    RGBA32( 10,  6,  3, 255)   /* warm near-black */
#define COL_DASH     RGBA32(  5,  3,  1, 255)   /* darker dashboard surface */
#define COL_CYAN     RGBA32(  0,179,217, 255)   /* indicator nominal */
#define COL_RED      RGBA32(217, 13, 13, 255)   /* indicator critical */
#define COL_IND_OFF  RGBA32(  5,  3,  2, 255)   /* indicator dead */
#define COL_SCR_BG   RGBA32(  4,  8, 17, 255)   /* signal screen interior */
#define COL_GLOVE    RGBA32( 11, 11, 14, 255)   /* glove body */
#define COL_CUFF     RGBA32(  8,  7, 10, 255)   /* cuff band */

void cockpit_draw(float oxygen_level) {
    /* --- structural frame --- */
    rdpq_set_mode_fill(COL_FRAME);
    rdpq_fill_rectangle(PIL_L_X1, 0,       PIL_L_X2, 240);      /* left pillar  */
    rdpq_fill_rectangle(PIL_R_X1, 0,       PIL_R_X2, 240);      /* right pillar */
    rdpq_fill_rectangle(INNER_X1, BAR_TOP_Y1, INNER_X2, BAR_TOP_Y2); /* top bar */
    rdpq_fill_rectangle(INNER_X1, MID_Y1,  INNER_X2, MID_Y2);  /* mid divider  */

    /* dashboard panel */
    rdpq_set_mode_fill(COL_DASH);
    rdpq_fill_rectangle(INNER_X1, DASH_Y1, INNER_X2, DASH_Y2);

    /* --- indicator cluster (oxygen / hull state) --- */
    int lit = (int)ceilf(oxygen_level * (float)IND_COUNT);
    if (lit < 0)          lit = 0;
    if (lit > IND_COUNT)  lit = IND_COUNT;
    bool critical = (oxygen_level <= 0.25f);

    for (int i = 0; i < IND_COUNT; i++) {
        int x1 = IND_X0 + i * IND_STEP;
        if (i < lit) {
            rdpq_set_mode_fill(critical ? COL_RED : COL_CYAN);
        } else {
            rdpq_set_mode_fill(COL_IND_OFF);
        }
        rdpq_fill_rectangle(x1, IND_Y1, x1 + IND_SQ, IND_Y2);
    }

    /* --- signal screen --- */
    rdpq_set_mode_fill(COL_FRAME);
    rdpq_fill_rectangle(SCR_BOR_X1, SCR_BOR_Y1, SCR_BOR_X2, SCR_BOR_Y2);
    rdpq_set_mode_fill(COL_SCR_BG);
    rdpq_fill_rectangle(SCR_X1, SCR_Y1, SCR_X2, SCR_Y2);
    rdpq_set_mode_fill(COL_CYAN);
    rdpq_fill_rectangle(DOT_X1, DOT_Y1, DOT_X2, DOT_Y2);

    /* --- gloved hand --- */
    rdpq_set_mode_fill(COL_GLOVE);
    rdpq_fill_rectangle(SLEEVE_X1, SLEEVE_Y1, SLEEVE_X2, SLEEVE_Y2);
    rdpq_fill_rectangle(THUMB_X1,  THUMB_Y1,  THUMB_X2,  THUMB_Y2);
    rdpq_set_mode_fill(COL_CUFF);
    rdpq_fill_rectangle(SLEEVE_X1, CUFF_Y1,   SLEEVE_X2, CUFF_Y2);
}
