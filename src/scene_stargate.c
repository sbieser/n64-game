/*
 * scene_stargate.c — 2001-style tunnel effect
 *
 * Inspired by the "Jupiter and Beyond the Infinite" sequence from
 * 2001: A Space Odyssey, created by Douglas Trumbull using slit-scan
 * photography. Each frame of film was exposed while the camera tracked
 * toward a moving slit, stretching artwork into infinite receding lines.
 * The result: two walls of colored light converging at a vanishing point,
 * palette cycling continuously from electric blue to orange to acid green.
 *
 * On N64 this is 8 rectangular frames at increasing Z depths. Each frame
 * is 4 thin quads (top/bottom/left/right strips). Every frame they advance
 * toward the camera; when one passes the near plane it wraps to the far
 * distance and inherits the next palette color. Total cost: 64 triangles,
 * one color table lookup per frame tick. Essentially free.
 *
 * In the final game this effect is the act-boundary transition — the player
 * crosses into it after Act 1, emerges into Act 2's stranger environment.
 * It can also be the visual signature of a warp speed boost.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "scene.h"

#define NUM_FRAMES     8
#define CORRIDOR_W    32.0f   /* half-width of the tunnel  */
#define CORRIDOR_H    20.0f   /* half-height of the tunnel */
#define STRIP_T        5.0f   /* thickness of each border strip */
#define Z_FAR        280.0f   /* Z where wrapped frames are born */
#define Z_NEAR         8.0f   /* Z where frames wrap back        */
#define SPEED          4.5f   /* world units per frame           */
#define COLOR_PERIOD   4      /* frames between palette advances */

static const uint32_t palette[8] = {
    0x0033FFFF,   /* electric blue  */
    0x00AAFFFF,   /* cyan-blue      */
    0x4400CCFF,   /* deep violet    */
    0xFF6600FF,   /* hot orange     */
    0xFFDD00FF,   /* acid yellow    */
    0xCC00FFFF,   /* magenta-purple */
    0x00FF88FF,   /* bright green   */
    0xFF0033FF,   /* hot red-pink   */
};

static T3DViewport    viewport;
static T3DMat4FP     *identMat;
static T3DVertPacked *frameBuf;  /* NUM_FRAMES * 8 packed structs */
static bool           initialized = false;

static float frame_z[NUM_FRAMES];
static int   palette_offset = 0;
static int   color_tick     = 0;

/* Fill two packed structs describing one axis-aligned quad at depth z.
 * Vertices: (x0,y0) = bottom-left, (x1,y1) = top-right. */
static void fill_quad(T3DVertPacked *b, int base,
                      float x0, float y0, float x1, float y1, float z,
                      uint32_t col) {
    b[base]   = (T3DVertPacked){
        .posA={(int16_t)x0,(int16_t)y0,(int16_t)z}, .rgbaA=col,
        .posB={(int16_t)x1,(int16_t)y0,(int16_t)z}, .rgbaB=col,
    };
    b[base+1] = (T3DVertPacked){
        .posA={(int16_t)x1,(int16_t)y1,(int16_t)z}, .rgbaA=col,
        .posB={(int16_t)x0,(int16_t)y1,(int16_t)z}, .rgbaB=col,
    };
}

void scene_stargate_init(void) {
    if (!initialized) {
        frameBuf = malloc_uncached(sizeof(T3DVertPacked) * NUM_FRAMES * 8);
        identMat = malloc_uncached(sizeof(T3DMat4FP));
        t3d_mat4fp_from_srt_euler(identMat,
            (float[3]){1,1,1}, (float[3]){0,0,0}, (float[3]){0,0,0});
        viewport = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 5.0f, 300.0f);
        initialized = true;
    }
    /* Evenly spread frames from near to far so no bunching on first frame */
    for (int i = 0; i < NUM_FRAMES; i++)
        frame_z[i] = Z_NEAR + (Z_FAR - Z_NEAR) * (float)(NUM_FRAMES - 1 - i) / (NUM_FRAMES - 1);
    palette_offset = 0;
    color_tick     = 0;
}

void scene_stargate_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    /* Advance all frames toward camera; wrap the ones that pass the near plane */
    for (int i = 0; i < NUM_FRAMES; i++) {
        frame_z[i] -= SPEED;
        if (frame_z[i] < Z_NEAR) frame_z[i] = Z_FAR;
    }

    /* Slowly cycle through the Kubrick palette */
    if (++color_tick % COLOR_PERIOD == 0)
        palette_offset = (palette_offset + 1) % 8;

    T3DVec3 camPos    = {{0.0f, 0.0f, -10.0f}};
    T3DVec3 camTarget = {{0.0f, 0.0f, 100.0f}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});
}

void scene_stargate_draw(void) {
    /* Rebuild vertex positions — Z values change every frame */
    float W = CORRIDOR_W, H = CORRIDOR_H, T = STRIP_T;
    for (int i = 0; i < NUM_FRAMES; i++) {
        float    z   = frame_z[i];
        uint32_t col = palette[(i + palette_offset) % 8];
        int      b   = i * 8;
        fill_quad(frameBuf, b+0, -W,   H,     W,    H+T,  z, col);  /* top    */
        fill_quad(frameBuf, b+2, -W,  -H-T,   W,   -H,    z, col);  /* bottom */
        fill_quad(frameBuf, b+4, -W-T,-H-T,  -W,   H+T,   z, col);  /* left   */
        fill_quad(frameBuf, b+6,  W,  -H-T,   W+T,  H+T,  z, col);  /* right  */
    }

    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

    t3d_screen_clear_color(RGBA32(0, 0, 0, 0xFF));
    t3d_screen_clear_depth();

    /* NO_LIGHT: use raw vertex colors so palette shows at full saturation */
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_NO_LIGHT);
    t3d_light_set_count(0);

    t3d_matrix_push(identMat);
    for (int i = 0; i < NUM_FRAMES; i++) {
        /* Load all 4 quads of one frame (16 verts) into RSP cache at once */
        t3d_vert_load(frameBuf + i * 8, 0, 16);
        for (int q = 0; q < 4; q++) {
            int v = q * 4;
            t3d_tri_draw(v, v+1, v+2);
            t3d_tri_draw(v, v+2, v+3);
        }
    }
    t3d_tri_sync();
    t3d_matrix_pop(1);

    rdpq_text_print(NULL, 1, 8, 228, "B back");
    rdpq_detach_show();
}
