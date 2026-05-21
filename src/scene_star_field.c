/*
 * scene_star_field.c — Stage 2: The Wake (young star field), free-roam
 *
 * The player moves through a volume of space 3,000 wide × 2,000 tall ×
 * 3,200 deep (Z: 0 to −3,200). Motion is free — the player controls X, Y,
 * and Z with the stick and buttons. No automatic forward pull. The ghost_reacher
 * sits at Z −2,850, past the five proto-stars.
 *
 * CONTROLS
 * ────────
 * Stick X/Y: lateral and vertical steering (add velocity to position)
 * Z button: thrust forward (−Z)
 * R button: brake / thrust backward (+Z)
 * B: return to scene select
 *
 * Velocity decays each frame (drag) so the player coasts to a stop after
 * releasing the stick. This matters for the "stop and rotate to seek" loop
 * that will eventually be the core mechanic — the player must be able to
 * hold still.
 *
 * BACKFACE CULLING
 * ─────────────────
 * The nebula quads are flat planes in world space. Depending on the player's
 * approach angle, the camera may be on either side of a quad's normal.
 * T3D_FLAG_CULL_BACK would hide quads viewed from behind, leaving invisible
 * holes in the nebula field. The star field draws WITHOUT CULL_BACK so all
 * geometry is visible from any direction. Proto-star cores (spheres) have
 * outward normals so they're fine either way.
 *
 * FAR CLIP AND DEPTH
 * ──────────────────
 * The stage volume is 3,200 units deep. Nebula quads sit at 1,500–3,000 units.
 * Far clip of 3,500 ensures everything is visible from the spawn point at Z=0
 * without clipping the deepest nebula. The ghost_reacher at Z −2,850 also
 * stays inside the far clip. Open space has no geometry to occlude, so a long
 * far clip costs nothing in overdraw — but it does affect Z-buffer precision.
 * Near clip of 2 is tighter than the corridor scene because the space is open
 * and the player may fly up to stars without hard walls stopping them.
 *
 * SOFT BOUNDS
 * ────────────
 * The stage has no physical walls. Soft bounds clamp the player back when they
 * drift past the stage volume edges — the position snaps to the boundary and the
 * velocity on that axis is zeroed. This prevents flying off to infinity with no
 * way back.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "scene.h"

#define BOUND_X      1500.0f
#define BOUND_Y      1000.0f
#define BOUND_Z_FAR -3200.0f
#define BOUND_Z_NEAR  200.0f   /* don't let player fly behind spawn */
#define THRUST        0.18f    /* units added to velocity per frame of input */
#define DRAG          0.92f    /* velocity multiplied each frame — coasts to stop */
#define MAX_SPEED     8.0f     /* terminal velocity per axis */
#define CAM_BACK      30.0f    /* camera trails behind the player */
#define CAM_RISE       5.0f

static T3DViewport  viewport;
static T3DModel    *model;
static T3DMat4FP   *modelMat;   /* [3] triple-buffered */
static float        posX, posY, posZ;
static float        velX, velY, velZ;
static int          frameIdx    = 0;
static bool         initialized = false;

static inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

void scene_star_field_init(void) {
    if (!initialized) {
        model    = t3d_model_load("rom:/young_star_field.t3dm");
        modelMat = malloc_uncached(sizeof(T3DMat4FP) * 3);

        float one[3]  = {1.0f, 1.0f, 1.0f};
        float zero[3] = {0.0f, 0.0f, 0.0f};
        for (int i = 0; i < 3; i++)
            t3d_mat4fp_from_srt_euler(&modelMat[i], one, zero, zero);

        viewport = t3d_viewport_create();
        /* Far clip 3500: captures nebula quads at 3000 units and the ghost_reacher
         * at 2850 units from anywhere in the stage. Open space has no overdraw
         * problem with a long far clip, but Z precision degrades. Near clip 2
         * keeps good precision for close star geometry. */
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 2.0f, 3500.0f);

        initialized = true;
    }

    posX = 0.0f; posY = 0.0f; posZ = 0.0f;
    velX = 0.0f; velY = 0.0f; velZ = 0.0f;
    frameIdx = 0;
}

void scene_star_field_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    joypad_inputs_t pad = joypad_get_inputs(JOYPAD_PORT_1);

    /* Stick X → lateral thrust; stick Y → vertical thrust.
     * Normalizing by 85 maps the stick's ~85-unit max to [-1, +1]. */
    velX += (pad.stick_x / 85.0f) * THRUST;
    velY += (pad.stick_y / 85.0f) * THRUST;

    /* Z button = thrust into the stage (−Z direction).
     * R button = thrust back toward start (+Z direction). */
    joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);
    if (held.z) velZ -= THRUST;
    if (held.r) velZ += THRUST;

    /* Drag — velocity decays toward zero each frame so the ship coasts to a stop.
     * Without drag, tiny stick inputs would accumulate forever. */
    velX *= DRAG;
    velY *= DRAG;
    velZ *= DRAG;
    velX = clampf(velX, -MAX_SPEED, MAX_SPEED);
    velY = clampf(velY, -MAX_SPEED, MAX_SPEED);
    velZ = clampf(velZ, -MAX_SPEED, MAX_SPEED);

    posX += velX;
    posY += velY;
    posZ += velZ;

    /* Soft bounds: clamp position and zero the corresponding velocity so the
     * player doesn't fight a spring force to stay in-bounds. */
    if (posX >  BOUND_X)    { posX =  BOUND_X;    velX = 0.0f; }
    if (posX < -BOUND_X)    { posX = -BOUND_X;    velX = 0.0f; }
    if (posY >  BOUND_Y)    { posY =  BOUND_Y;    velY = 0.0f; }
    if (posY < -BOUND_Y)    { posY = -BOUND_Y;    velY = 0.0f; }
    if (posZ >  BOUND_Z_NEAR) { posZ =  BOUND_Z_NEAR; velZ = 0.0f; }
    if (posZ <  BOUND_Z_FAR)  { posZ =  BOUND_Z_FAR;  velZ = 0.0f; }

    /* Camera trails behind the player and looks toward where they're heading.
     * The velocity lean (velX * 2, velY * 2) tilts the look-at target slightly
     * in the direction of motion — subtle banking without actual ship tilt. */
    T3DVec3 camPos    = {{posX, posY + CAM_RISE, posZ + CAM_BACK}};
    T3DVec3 camTarget = {{posX + velX * 2.0f, posY + velY * 2.0f, posZ - 50.0f}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});
}

void scene_star_field_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);

    /* Near-black void: very faint blue tint so pure black stars read against it.
     * The nebula quads and proto-stars provide all the light. */
    t3d_screen_clear_color(RGBA32(2, 2, 8, 0xFF));
    t3d_screen_clear_depth();

    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
    /* No CULL_BACK: nebula quads are flat planes that must be visible from
     * either side regardless of player orientation. */
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_NO_LIGHT);

    t3d_matrix_push(&modelMat[frameIdx]);
    t3d_model_draw(model);
    t3d_matrix_pop(1);

    rdpq_detach_show();
    frameIdx = (frameIdx + 1) % 3;
}
