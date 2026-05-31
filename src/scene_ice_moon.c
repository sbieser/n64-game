/*
 * scene_ice_moon.c — Stage 3: Ice Moon environmental rails
 *
 * The player descends through an ice moon corridor from Z=0 to Z=−5000.
 * Forward motion is automatic — 7 world units per second (0.117 per frame
 * at 60fps). The stick steers left/right (X) and up/down (Y) within the
 * channel walls. The player cannot stop or reverse; this is the "environmental
 * rails" mode where the landscape channels rather than forces movement.
 *
 * MODEL AND SCALE
 * ───────────────
 * ice_moon.t3dm was built in Blender at world scale — 1 Blender unit = 1 game
 * unit = 1 meter. The model matrix is identity (scale 1, no rotation, no
 * translation). The corridor geometry sits at its real-world positions in the
 * coordinate system: walls at X ±22, floor at Y −12, ceiling varying by zone.
 *
 * The model was exported with --ignore-materials. This writes dummy zero data
 * for all material fields. When t3d_model_draw processes each mesh, it compares
 * the material's colorCombiner (0) against the last-set value. If both are 0,
 * no combiner update is issued and our pre-set RDPQ_COMBINER_SHADE remains
 * active for the entire draw. This is how --ignore-materials enables a custom
 * material system: set your state before the draw, and the model won't disturb it.
 *
 * VERTEX COLORS AND NO_LIGHT
 * ──────────────────────────
 * All geometry in the model carries vertex colors (baked in Blender): cold
 * blue-white for ice, warm amber for geothermal vents, warm orange for the gas
 * giant billboard. T3D_FLAG_NO_LIGHT disables the normal-based lighting
 * calculation and renders the raw vertex color directly. This gives full control
 * over the look without configuring directional lights that the cave geometry
 * would shadow incorrectly anyway.
 *
 * T3D_FLAG_CULL_BACK skips triangles whose normals face away from the camera.
 * The corridor walls were built with normals facing inward (toward the player),
 * so backface culling is safe and correct here — it halves the draw cost for
 * every wall slab.
 *
 * MATRIX TRIPLE-BUFFERING
 * ────────────────────────
 * t3d_matrix_push() sends the matrix pointer to the RSP, which reads it via
 * DMA asynchronously — after the CPU has already moved on. If we used one matrix
 * and overwrote it each frame, the RSP might read mid-update. Three slots indexed
 * by display_get_index() (which cycles 0/1/2 with the framebuffers) guarantee the
 * RSP and CPU never touch the same slot simultaneously.
 *
 * The identity matrix never changes, so we compute all three slots once during
 * init and never touch them again.
 */

#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "scene.h"
#include "cockpit.h"

#define RAIL_START       0.0f
#define RAIL_END      -5000.0f
#define FORWARD_SPEED    0.117f    /* 7 units/second at 60fps */
#define LATERAL_MAX     22.0f
#define VERTICAL_MAX    12.0f
#define CAM_BACK        20.0f     /* camera trails this many units behind rail */
#define CAM_RISE         8.0f     /* camera sits above the player's eye height */
#define CAM_LOOK_AHEAD  10.0f     /* camera target is this many units ahead */

static T3DViewport  viewport;
static T3DModel    *model;
static T3DMat4FP   *modelMat;   /* [3] — triple-buffered identity matrix */
static float        railZ, lateralPos, verticalPos;
static float        camPosX, camPosY, camPosZ;
static float        lookX, lookY, lookZ;
static int          frameIdx    = 0;
static bool         initialized = false;

void scene_ice_moon_init(void) {
    if (!initialized) {
        model    = t3d_model_load("rom:/ice_moon.t3dm");
        modelMat = malloc_uncached(sizeof(T3DMat4FP) * 3);

        /* Identity matrix: scale 1, no rotation, no translation.
         * Computed once into all three triple-buffer slots. */
        float one[3] = {1.0f, 1.0f, 1.0f};
        float zero[3] = {0.0f, 0.0f, 0.0f};
        for (int i = 0; i < 3; i++)
            t3d_mat4fp_from_srt_euler(&modelMat[i], one, zero, zero);

        viewport = t3d_viewport_create();
        /* Far clip 1000: terrain chunks draw to 800 units, gas giant to 3000
         * but the gas giant billboard is only visible in zone 0 (first 1200 units).
         * 1000 captures all active geometry without wasting fill rate on sky. */
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), 5.0f, 1000.0f);

        initialized = true;
    }

    railZ       = RAIL_START;
    lateralPos  = 0.0f;
    verticalPos = 0.0f;
    camPosX = 0.0f; camPosY = CAM_RISE; camPosZ = RAIL_START + CAM_BACK;
    lookX = 0.0f; lookY = 0.0f; lookZ = -1.0f;
    frameIdx    = 0;
}

void scene_ice_moon_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    joypad_inputs_t pad = joypad_get_inputs(JOYPAD_PORT_1);
    lateralPos  += (pad.stick_x / 85.0f) * 0.4f;
    verticalPos += (pad.stick_y / 85.0f) * 0.4f;
    if (lateralPos  >  LATERAL_MAX)  lateralPos  =  LATERAL_MAX;
    if (lateralPos  < -LATERAL_MAX)  lateralPos  = -LATERAL_MAX;
    if (verticalPos >  VERTICAL_MAX) verticalPos =  VERTICAL_MAX;
    if (verticalPos < -VERTICAL_MAX) verticalPos = -VERTICAL_MAX;

    railZ -= FORWARD_SPEED;
    if (railZ < RAIL_END) railZ = RAIL_END;

    /* Camera trails behind the player and looks slightly forward. The slight
     * lean of camTarget toward the player's lateral/vertical position creates
     * a banking feel — the view tilts into the direction of movement. */
    camPosX = lateralPos;
    camPosY = verticalPos + CAM_RISE;
    camPosZ = railZ + CAM_BACK;
    T3DVec3 camPos    = {{camPosX, camPosY, camPosZ}};
    T3DVec3 camTarget = {{lateralPos + lateralPos * 0.3f,
                          verticalPos + verticalPos * 0.2f,
                          railZ - CAM_LOOK_AHEAD}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});

    /* Normalize look direction for cockpit (camTarget - camPos, then normalize). */
    lookX = camTarget.v[0] - camPosX;
    lookY = camTarget.v[1] - camPosY;
    lookZ = camTarget.v[2] - camPosZ;
    float invLen = 1.0f / sqrtf(lookX*lookX + lookY*lookY + lookZ*lookZ);
    lookX *= invLen; lookY *= invLen; lookZ *= invLen;
}

void scene_ice_moon_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    /* Cold dark blue — the void between ice walls. Vents and vertex colors
     * provide all the warmth; the background should not compete. */
    t3d_screen_clear_color(RGBA32(5, 8, 20, 0xFF));
    t3d_screen_clear_depth();

    /* Set combiner and draw flags before the model draw. The --ignore-materials
     * dummy data (all zeros) leaves these untouched throughout the entire draw. */
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH |
                            T3D_FLAG_CULL_BACK | T3D_FLAG_NO_LIGHT);

    t3d_matrix_push(&modelMat[frameIdx]);
    t3d_model_draw(model);
    t3d_matrix_pop(1);

    cockpit_draw_frame(camPosX, camPosY, camPosZ, lookX, lookY, lookZ);
    cockpit_draw_hud(1.0f, 0.0f, 0.0f);   /* TODO: wire hull_temp_level from game state */

    rdpq_detach_show();
    frameIdx = (frameIdx + 1) % 3;
}
