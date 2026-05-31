/*
 * scene_star_field.c — Stage 2: The Wake (young star field), free-roam
 *
 * The player moves through a volume of space 3,000 wide × 2,000 tall ×
 * 3,200 deep (Z: 0 to −3,200). Motion is free with flight-sim controls:
 * the stick steers heading (yaw/pitch) and the Z/R buttons thrust along
 * the look direction. Velocity persists with drag so the player coasts to
 * a stop after releasing thrust — enabling the "stop and rotate to seek"
 * loop that is the core mechanic. The ghost_reacher sits at Z −2,850.
 *
 * CONTROLS
 * ────────
 * Stick X:  yaw left/right
 * Stick Y:  pitch up/down
 * Z button: thrust forward (along look direction)
 * R button: brake / thrust backward
 * B:        return to scene select
 *
 * LOOK DIRECTION
 * ──────────────
 * Derived from yaw and pitch each frame:
 *   lookX =  sin(yaw) * cos(pitch)
 *   lookY =  sin(pitch)
 *   lookZ = -cos(yaw) * cos(pitch)
 * At yaw=0, pitch=0 this gives (0,0,−1) — looking into the scene.
 * Pitch is clamped to ±80° to prevent gimbal flip at the poles.
 *
 * CAMERA
 * ──────
 * First-person: camera sits at the player position looking along lookDir.
 * The cockpit overlay frames the view. World up (0,1,0) is the up vector;
 * no roll is implemented, keeping the horizon stable.
 *
 * VELOCITY
 * ─────────
 * Thrust adds to a world-space velocity vector along the current look
 * direction. Drag decays all three components each frame. Speed is clamped
 * by vector magnitude (not per-axis) so the terminal speed is consistent
 * regardless of heading. Turning while moving causes a realistic drift —
 * the ship slides through space until drag kills the old velocity.
 *
 * BACKFACE CULLING
 * ─────────────────
 * The nebula quads are flat planes in world space. Depending on the player's
 * approach angle, the camera may be on either side of a quad's normal.
 * T3D_FLAG_CULL_BACK would hide quads viewed from behind, leaving invisible
 * holes in the nebula field. The star field draws WITHOUT CULL_BACK so all
 * geometry is visible from any direction.
 *
 * SOFT BOUNDS
 * ────────────
 * The stage has no physical walls. Soft bounds clamp the player back when
 * they drift past the stage volume edges — position snaps to the boundary
 * and velocity on that axis is zeroed.
 */

#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "scene.h"
#include "cockpit.h"

#define BOUND_X       1500.0f
#define BOUND_Y       1000.0f
#define BOUND_Z_FAR  -3200.0f
#define BOUND_Z_NEAR   200.0f
#define THRUST          0.18f   /* velocity added per frame of thrust input */
#define DRAG            0.92f   /* velocity multiplied each frame */
#define MAX_SPEED        8.0f   /* terminal speed (vector magnitude) */
#define YAW_RATE        0.030f  /* radians per frame at full stick deflection */
#define PITCH_RATE      0.025f  /* radians per frame at full stick deflection */
#define PITCH_MAX       1.396f  /* 80° in radians — avoids gimbal flip */

static T3DViewport  viewport;
static T3DModel    *model;
static T3DMat4FP   *modelMat;   /* [3] triple-buffered */
static float        posX, posY, posZ;
static float        velX, velY, velZ;
static float        yaw, pitch;
static int          frameIdx    = 0;
static bool         initialized = false;

void scene_star_field_init(void) {
    if (!initialized) {
        model    = t3d_model_load("rom:/young_star_field.t3dm");
        modelMat = malloc_uncached(sizeof(T3DMat4FP) * 3);

        float one[3]  = {1.0f, 1.0f, 1.0f};
        float zero[3] = {0.0f, 0.0f, 0.0f};
        for (int i = 0; i < 3; i++)
            t3d_mat4fp_from_srt_euler(&modelMat[i], one, zero, zero);

        viewport = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), 2.0f, 3500.0f);

        initialized = true;
    }

    posX = 0.0f; posY = 0.0f; posZ = 0.0f;
    velX = 0.0f; velY = 0.0f; velZ = 0.0f;
    yaw  = 0.0f; pitch = 0.0f;
    frameIdx = 0;
}

void scene_star_field_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    joypad_inputs_t pad  = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);

    /* Stick steers heading. Normalizing by 85 maps stick range to [−1, +1]. */
    yaw   += (pad.stick_x / 85.0f) * YAW_RATE;
    pitch += (pad.stick_y / 85.0f) * PITCH_RATE;
    if (pitch >  PITCH_MAX) pitch =  PITCH_MAX;
    if (pitch < -PITCH_MAX) pitch = -PITCH_MAX;

    /* Look direction vector from yaw and pitch. */
    float lookX =  sinf(yaw) * cosf(pitch);
    float lookY =  sinf(pitch);
    float lookZ = -cosf(yaw) * cosf(pitch);

    /* Thrust adds velocity along the current look direction. */
    if (held.z) { velX += lookX * THRUST; velY += lookY * THRUST; velZ += lookZ * THRUST; }
    if (held.r) { velX -= lookX * THRUST; velY -= lookY * THRUST; velZ -= lookZ * THRUST; }

    /* Drag decays velocity toward zero each frame. */
    velX *= DRAG;
    velY *= DRAG;
    velZ *= DRAG;

    /* Clamp by vector magnitude so terminal speed is consistent in all directions. */
    float speed2 = velX*velX + velY*velY + velZ*velZ;
    if (speed2 > MAX_SPEED * MAX_SPEED) {
        float inv = MAX_SPEED / sqrtf(speed2);
        velX *= inv; velY *= inv; velZ *= inv;
    }

    posX += velX;
    posY += velY;
    posZ += velZ;

    /* Soft bounds: snap to edge and kill velocity on that axis. */
    if (posX >  BOUND_X)     { posX =  BOUND_X;     velX = 0.0f; }
    if (posX < -BOUND_X)     { posX = -BOUND_X;     velX = 0.0f; }
    if (posY >  BOUND_Y)     { posY =  BOUND_Y;     velY = 0.0f; }
    if (posY < -BOUND_Y)     { posY = -BOUND_Y;     velY = 0.0f; }
    if (posZ >  BOUND_Z_NEAR){ posZ =  BOUND_Z_NEAR; velZ = 0.0f; }
    if (posZ <  BOUND_Z_FAR) { posZ =  BOUND_Z_FAR;  velZ = 0.0f; }

    /* First-person camera: sit at player position, look along heading. */
    T3DVec3 camPos    = {{posX, posY, posZ}};
    T3DVec3 camTarget = {{posX + lookX, posY + lookY, posZ + lookZ}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});
}

void scene_star_field_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    t3d_screen_clear_color(RGBA32(2, 2, 8, 0xFF));
    t3d_screen_clear_depth();

    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_NO_LIGHT);

    t3d_matrix_push(&modelMat[frameIdx]);
    t3d_model_draw(model);
    t3d_matrix_pop(1);

    cockpit_draw_frame(posX, posY, posZ,
                       sinf(yaw) * cosf(pitch),
                       sinf(pitch),
                      -cosf(yaw) * cosf(pitch));
    cockpit_draw_hud(1.0f, 0.0f, 0.0f, 0.0f);   /* TODO: wire oxygen_level from game state */

    rdpq_detach_show();
    frameIdx = (frameIdx + 1) % 3;
}
