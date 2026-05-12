/**
 * N64 Learn — Rails with Obstacles
 *
 * Entry point and game loop. The camera flies forward on a fixed rail;
 * the player steers with the analog stick to avoid obstacle shapes.
 * See shapes.c for geometry, obstacles.c for placement and collision.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "shapes.h"
#include "obstacles.h"
#include "starfield.h"
#include "player.h"

#define RAIL_START      -250.0f
#define RAIL_END          50.0f
#define LATERAL_MAX       20.0f
#define VERTICAL_MAX       6.0f
#define HIT_FLASH_FRAMES  30

int main(void) {
    debug_init_isviewer();
    debug_init_usblog();

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
    t3d_init((T3DInitParams){});
    joypad_init();

    shapes_init();
    obstacles_init();
    starfield_init(RAIL_START, RAIL_END);
    player_init();

    T3DViewport viewport = t3d_viewport_create();

    uint8_t ambientColor[4]     = {40,  40,  80, 0xFF};
    uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
    T3DVec3 lightDir = {{1.0f, 1.0f, -0.5f}};
    t3d_vec3_norm(&lightDir);

    float railZ       = RAIL_START;
    float rotAngle    = 0.0f;
    float lateralPos  = 0.0f;
    float verticalPos = 0.0f;
    int   hitFlash    = 0;

    for (;;) {
        /* ---- UPDATE ---- */

        joypad_poll();
        joypad_inputs_t pad = joypad_get_inputs(JOYPAD_PORT_1);

        /* Velocity-based steering: holding the stick accelerates the camera
         * sideways each frame rather than snapping to a fixed offset. */
        lateralPos  += (pad.stick_x / 85.0f) * 0.5f;
        verticalPos += (pad.stick_y / 85.0f) * 0.5f;
        if (lateralPos  >  LATERAL_MAX)  lateralPos  =  LATERAL_MAX;
        if (lateralPos  < -LATERAL_MAX)  lateralPos  = -LATERAL_MAX;
        if (verticalPos >  VERTICAL_MAX) verticalPos =  VERTICAL_MAX;
        if (verticalPos < -VERTICAL_MAX) verticalPos = -VERTICAL_MAX;

        railZ    += 0.4f;
        rotAngle += 0.02f;
        if (railZ > RAIL_END) railZ = RAIL_START;

        obstacles_update(rotAngle);
        player_update(lateralPos, verticalPos, railZ);

        /* Camera sits 15 units behind railZ. */
        float camZ = railZ - 15.0f;

        if (hitFlash > 0) {
            hitFlash--;
        } else if (obstacles_check_collision(camZ, lateralPos, verticalPos)) {
            hitFlash = HIT_FLASH_FRAMES;
        }

        /* Camera follows the stick. The look-at target uses smaller lateral
         * multipliers so the camera tilts into turns rather than sliding flat. */
        T3DVec3 camPos    = {{lateralPos,        8.0f + verticalPos, camZ}};
        T3DVec3 camTarget = {{lateralPos * 0.6f, verticalPos * 0.4f, railZ + 10.0f}};

        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 10.0f, 200.0f);
        t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

        /* ---- DRAW ---- */

        rdpq_attach(display_get(), display_get_zbuf());
        t3d_frame_start();
        t3d_viewport_attach(&viewport);
        rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

        /* Flash red on hit by changing the background clear color. */
        t3d_screen_clear_color(hitFlash > 0
            ? RGBA32(100, 20, 20, 0xFF)
            : RGBA32( 50, 50,100, 0xFF));
        t3d_screen_clear_depth();

        /* Directional lights must be re-applied after each t3d_viewport_attach
         * because the view matrix affects how the RSP transforms them. */
        t3d_light_set_ambient(ambientColor);
        t3d_light_set_directional(0, directionalColor, &lightDir);
        t3d_light_set_count(1);

        /* Stars render without lighting — they glow at full vertex color.
         * Player and obstacles use lighting for solid 3D shading. */
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK | T3D_FLAG_NO_LIGHT);
        starfield_draw();

        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
        player_draw();
        obstacles_draw();

        /* Flip to screen. Syncs the full RSP/RDP pipeline — safe to
         * overwrite obstacle matrices on the next frame. */
        rdpq_detach_show();
    }

    t3d_destroy();
    return 0;
}
