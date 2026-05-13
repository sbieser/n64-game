#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "scene.h"
#include "shapes.h"
#include "obstacles.h"
#include "starfield.h"
#include "player.h"

#define RAIL_START      -600.0f
#define RAIL_END          50.0f
#define LATERAL_MAX       35.0f
#define VERTICAL_MAX      18.0f
#define HIT_FLASH_FRAMES  30

static T3DViewport viewport;
static float       railZ, rotAngle, lateralPos, verticalPos, camZ;
static int         hitFlash;
static bool        initialized = false;

static uint8_t ambientColor[4]     = {40,  40,  80, 0xFF};
static uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
static T3DVec3 lightDir;

void scene_rails_init(void) {
    if (!initialized) {
        shapes_init();
        obstacles_init();
        starfield_init(RAIL_START, RAIL_END);
        player_init();
        viewport = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 10.0f, 200.0f);
        lightDir = (T3DVec3){{1.0f, 1.0f, -0.5f}};
        t3d_vec3_norm(&lightDir);
        initialized = true;
    }
    railZ       = RAIL_START;
    rotAngle    = 0.0f;
    lateralPos  = 0.0f;
    verticalPos = 0.0f;
    hitFlash    = 0;
}

void scene_rails_update(void) {
    joypad_poll();
    joypad_inputs_t  pad = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    lateralPos  += (pad.stick_x / 85.0f) * 0.5f;
    verticalPos += (pad.stick_y / 85.0f) * 0.5f;
    if (lateralPos  >  LATERAL_MAX)  lateralPos  =  LATERAL_MAX;
    if (lateralPos  < -LATERAL_MAX)  lateralPos  = -LATERAL_MAX;
    if (verticalPos >  VERTICAL_MAX) verticalPos =  VERTICAL_MAX;
    if (verticalPos < -VERTICAL_MAX) verticalPos = -VERTICAL_MAX;

    railZ    += 0.4f;
    rotAngle += 0.02f;
    if (railZ > RAIL_END) railZ = RAIL_START;

    camZ = railZ - 15.0f;
    float playerZ = railZ - 3.0f;

    obstacles_update(rotAngle, camZ);
    player_update(lateralPos, verticalPos, railZ);

    if (hitFlash > 0) {
        hitFlash--;
    } else if (obstacles_check_collision(playerZ, lateralPos, verticalPos)) {
        hitFlash = HIT_FLASH_FRAMES;
    }

    T3DVec3 camPos    = {{lateralPos,        8.0f + verticalPos, camZ}};
    T3DVec3 camTarget = {{lateralPos * 0.6f, verticalPos * 0.4f, railZ + 10.0f}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
}

void scene_rails_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

    t3d_screen_clear_color(hitFlash > 0
        ? RGBA32(100, 20, 20, 0xFF)
        : RGBA32( 50, 50,100, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(ambientColor);
    t3d_light_set_directional(0, directionalColor, &lightDir);
    t3d_light_set_count(1);

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK | T3D_FLAG_NO_LIGHT);
    starfield_draw();

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
    player_draw();
    obstacles_draw(camZ);

    rdpq_detach_show();
}
