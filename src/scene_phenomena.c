/*
 * scene_phenomena.c — Gravitational field test environment
 *
 * A stripped rails scene with no obstacles or death. The only active system
 * is phenomena — gravitational fields that push the player laterally. Use
 * this scene to tune field strength, length, and visual feedback before the
 * fields go into the live rails scene.
 *
 * The zone map HUD at the bottom shows the full rail length as a horizontal
 * bar. Coloured regions mark each gravitational field (orange = rightward
 * pull, blue = leftward). The white dot is your current position. This makes
 * it immediately clear when you are entering or leaving a field zone and how
 * far each zone extends.
 *
 * A button generates a new seeded run. B returns to the selector.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "scene.h"
#include "shapes.h"
#include "starfield.h"
#include "player.h"
#include "phenomena.h"

#define RAIL_START   -600.0f
#define RAIL_END       50.0f
#define LATERAL_MAX    35.0f
#define VERTICAL_MAX   18.0f

/* Zone map geometry — a horizontal strip at the bottom of the screen */
#define MAP_X      16
#define MAP_W     288
#define MAP_Y     222
#define MAP_H       6

static T3DViewport viewport;
static float       railZ, lateralPos, verticalPos, camZ;
static bool        initialized = false;
static uint32_t    seed;

static uint8_t ambientColor[4]     = {40,  40,  80, 0xFF};
static uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
static T3DVec3 lightDir;

void scene_phenomena_init(void) {
    seed = (uint32_t)TICKS_READ();

    if (!initialized) {
        shapes_init();
        starfield_init(RAIL_START, RAIL_END);
        player_init();
        viewport = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 10.0f, 200.0f);
        lightDir = (T3DVec3){{1.0f, 1.0f, -0.5f}};
        t3d_vec3_norm(&lightDir);
        initialized = true;
    }

    starfield_generate(seed);
    phenomena_generate(seed);

    railZ       = RAIL_START;
    lateralPos  = 0.0f;
    verticalPos = 0.0f;
}

void scene_phenomena_update(void) {
    joypad_poll();
    joypad_inputs_t  pad = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    if (btn.a) {
        seed = (uint32_t)TICKS_READ();
        starfield_generate(seed);
        phenomena_generate(seed);
        railZ = RAIL_START;
        lateralPos = verticalPos = 0.0f;
        return;
    }

    lateralPos  += (pad.stick_x / 85.0f) * 0.5f;
    verticalPos += (pad.stick_y / 85.0f) * 0.5f;
    phenomena_update(railZ, &lateralPos);
    if (lateralPos  >  LATERAL_MAX)  lateralPos  =  LATERAL_MAX;
    if (lateralPos  < -LATERAL_MAX)  lateralPos  = -LATERAL_MAX;
    if (verticalPos >  VERTICAL_MAX) verticalPos =  VERTICAL_MAX;
    if (verticalPos < -VERTICAL_MAX) verticalPos = -VERTICAL_MAX;

    railZ += 0.4f;
    if (railZ > RAIL_END) railZ = RAIL_START;

    camZ = railZ - 15.0f;

    T3DVec3 camPos    = {{lateralPos,        8.0f + verticalPos, camZ}};
    T3DVec3 camTarget = {{lateralPos * 0.6f, verticalPos * 0.4f, railZ + 10.0f}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});

    player_update(lateralPos, verticalPos, railZ);
}

void scene_phenomena_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

    /* Background tint: warmer when pulled right, cooler when pulled left */
    float pull = phenomena_pull();
    uint8_t bgR = 50 + (pull > 0.0f ? 30 : 0);
    uint8_t bgB = 100 + (pull < 0.0f ? 40 : 0);
    t3d_screen_clear_color(RGBA32(bgR, 50, bgB, 0xFF));
    t3d_screen_clear_depth();

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK | T3D_FLAG_NO_LIGHT);
    starfield_draw();

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
    t3d_light_set_ambient(ambientColor);
    t3d_light_set_directional(0, directionalColor, &lightDir);
    t3d_light_set_count(1);
    player_draw();

    /* --- HUD ------------------------------------------------------------ */

    /* Zone map background */
    rdpq_set_mode_fill(RGBA32(20, 20, 40, 0xFF));
    rdpq_fill_rectangle(MAP_X, MAP_Y, MAP_X + MAP_W, MAP_Y + MAP_H);

    /* Coloured bars for each grav field zone */
    float rail_len = RAIL_END - RAIL_START;
    int n = phenomena_field_count();
    for (int i = 0; i < n; i++) {
        float zs, ze, p;
        phenomena_get_field(i, &zs, &ze, &p);
        int x0 = MAP_X + (int)((zs - RAIL_START) / rail_len * MAP_W);
        int x1 = MAP_X + (int)((ze - RAIL_START) / rail_len * MAP_W);
        if (x0 < MAP_X) x0 = MAP_X;
        if (x1 > MAP_X + MAP_W) x1 = MAP_X + MAP_W;
        /* Orange = rightward pull, blue = leftward */
        if (p > 0.0f)
            rdpq_set_mode_fill(RGBA32(200, 100, 30, 0xFF));
        else
            rdpq_set_mode_fill(RGBA32(30, 80, 200, 0xFF));
        rdpq_fill_rectangle(x0, MAP_Y, x1, MAP_Y + MAP_H);
    }

    /* Player position dot */
    int px = MAP_X + (int)((railZ - RAIL_START) / rail_len * MAP_W);
    rdpq_set_mode_fill(RGBA32(255, 255, 255, 0xFF));
    rdpq_fill_rectangle(px - 1, MAP_Y - 1, px + 2, MAP_Y + MAP_H + 1);

    /* Status text */
    rdpq_text_print(NULL, 1, 8, 14, "PHENOMENA TEST");
    if (pull > 0.0f)
        rdpq_text_printf(NULL, 1, 8, 26, "GRAV: >>> RIGHT  %.3f/frame", pull);
    else if (pull < 0.0f)
        rdpq_text_printf(NULL, 1, 8, 26, "GRAV: <<< LEFT   %.3f/frame", -pull);
    else
        rdpq_text_print(NULL, 1, 8, 26, "GRAV: none");

    rdpq_text_print(NULL, 1, 8, 214, "ORANGE=right  BLUE=left  DOT=you");
    rdpq_text_print(NULL, 1, 8, 236, "A:new seed   B:back");

    rdpq_detach_show();
}
