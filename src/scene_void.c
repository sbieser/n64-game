/*
 * scene_void.c — Stage 1: The Void
 *
 * The opening stage. Deep space, no landmarks, no instruction. A signal
 * exists somewhere in the void — audio volume and pan will eventually
 * guide the player toward the Pioneer (a frozen beacon ghost). For now
 * the Pioneer is a placeholder shape at a fixed position.
 *
 * OXYGEN
 * ───────
 * Oxygen depletes only while thrusting (Z or R held). Rotation and
 * stillness cost nothing — the "stop, rotate, listen" loop is the
 * breathing space the mechanic creates. At zero the player dies: their
 * position is written to EEPROM as a ghost and play returns to select.
 * Future visits load those ghosts and render them as frozen wrecks.
 *
 * The cockpit indicator tracks oxygen via cockpit_draw(oxygen):
 *   ceil(oxygen * 8) squares lit cyan → red at ≤ 25% → all dark at 0.
 *
 * PIONEER
 * ────────
 * Static beacon at (PIONEER_X, PIONEER_Y, PIONEER_Z). Entering
 * PIONEER_RADIUS triggers stage completion (currently returns to select
 * until Stage 2 is wired). Rendered as a placeholder octahedron until
 * the humanoid model exists.
 *
 * CONTROLS
 * ─────────
 * Stick X/Y  yaw / pitch
 * Z          thrust forward — costs oxygen
 * R          thrust backward — costs oxygen
 * B          return to select
 */

#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "scene.h"
#include "cockpit.h"
#include "ghost.h"
#include "shapes.h"

/* Stage volume */
#define BOUND_X       2000.0f
#define BOUND_Y       1000.0f
#define BOUND_Z_FAR  -4000.0f
#define BOUND_Z_NEAR   200.0f

/* Flight */
#define THRUST          0.18f
#define DRAG            0.92f
#define MAX_SPEED        8.0f
#define YAW_RATE        0.030f
#define PITCH_RATE      0.025f
#define PITCH_MAX       1.396f    /* 80° in radians */

/* Oxygen depletes while thrusting only.
 * Full tank ≈ 90 seconds of continuous thrust at 60 fps. */
#define OXY_DRAIN_RATE  0.000185f

/* Pioneer beacon ghost — placeholder until humanoid model is ready */
#define PIONEER_X      200.0f
#define PIONEER_Y        0.0f
#define PIONEER_Z    -1800.0f
#define PIONEER_RADIUS  100.0f    /* proximity for stage completion */
#define PIONEER_SCALE   20.0f     /* placeholder scale — visible at distance */

/* Frames of red-screen death flash before returning to select */
#define DEATH_FRAMES    90

static T3DViewport  viewport;
static T3DMat4FP   *pioneerMat;   /* [3] triple-buffered — position never changes */
static float        posX, posY, posZ;
static float        velX, velY, velZ;
static float        yaw, pitch;
static float        lookX, lookY, lookZ;
static float        oxygen;
static int          deathTimer;
static bool         dead;
static uint32_t     frameCount;
static int          frameIdx;
static bool         initialized = false;

void scene_void_init(void) {
    if (!initialized) {
        shapes_init();
        ghost_init();

        pioneerMat = malloc_uncached(sizeof(T3DMat4FP) * 3);
        float scale[3] = {PIONEER_SCALE, PIONEER_SCALE, PIONEER_SCALE};
        float rot[3]   = {0.0f, 0.0f, 0.0f};
        float pos[3]   = {PIONEER_X, PIONEER_Y, PIONEER_Z};
        for (int i = 0; i < 3; i++)
            t3d_mat4fp_from_srt_euler(&pioneerMat[i], scale, rot, pos);

        viewport = t3d_viewport_create();
        /* Far clip 4000 matches BOUND_Z_FAR so nothing inside the stage
         * ever clips. Pioneer sits at 1800 — well inside range. */
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), 2.0f, 4000.0f);

        initialized = true;
    }

    ghost_load();

    posX = 0.0f; posY = 0.0f; posZ = 0.0f;
    velX = 0.0f; velY = 0.0f; velZ = 0.0f;
    yaw  = 0.0f; pitch  = 0.0f;
    lookX = 0.0f; lookY = 0.0f; lookZ = -1.0f;
    oxygen     = 1.0f;
    deathTimer = 0;
    dead       = false;
    frameCount = 0;
    frameIdx   = 0;
}

void scene_void_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    frameCount++;

    /* After death, count down the flash then return to select */
    if (dead) {
        if (--deathTimer <= 0) scene_switch(SCENE_SELECT);
        return;
    }

    joypad_inputs_t  pad  = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);

    /* Stick steers heading */
    yaw   += (pad.stick_x / 85.0f) * YAW_RATE;
    pitch += (pad.stick_y / 85.0f) * PITCH_RATE;
    if (pitch >  PITCH_MAX) pitch =  PITCH_MAX;
    if (pitch < -PITCH_MAX) pitch = -PITCH_MAX;

    /* Look direction from yaw and pitch — stored as statics for draw() */
    lookX =  sinf(yaw) * cosf(pitch);
    lookY =  sinf(pitch);
    lookZ = -cosf(yaw) * cosf(pitch);

    /* Thrust along look direction — costs oxygen */
    bool thrusting = held.z || held.r;
    if (held.z) { velX += lookX * THRUST; velY += lookY * THRUST; velZ += lookZ * THRUST; }
    if (held.r) { velX -= lookX * THRUST; velY -= lookY * THRUST; velZ -= lookZ * THRUST; }

    if (thrusting) {
        oxygen -= OXY_DRAIN_RATE;
        if (oxygen < 0.0f) oxygen = 0.0f;
    }

    /* Drag and speed cap */
    velX *= DRAG; velY *= DRAG; velZ *= DRAG;
    float spd2 = velX*velX + velY*velY + velZ*velZ;
    if (spd2 > MAX_SPEED * MAX_SPEED) {
        float inv = MAX_SPEED / sqrtf(spd2);
        velX *= inv; velY *= inv; velZ *= inv;
    }

    posX += velX; posY += velY; posZ += velZ;

    /* Soft bounds */
    if (posX >  BOUND_X)     { posX =  BOUND_X;     velX = 0.0f; }
    if (posX < -BOUND_X)     { posX = -BOUND_X;     velX = 0.0f; }
    if (posY >  BOUND_Y)     { posY =  BOUND_Y;     velY = 0.0f; }
    if (posY < -BOUND_Y)     { posY = -BOUND_Y;     velY = 0.0f; }
    if (posZ >  BOUND_Z_NEAR){ posZ =  BOUND_Z_NEAR; velZ = 0.0f; }
    if (posZ <  BOUND_Z_FAR) { posZ =  BOUND_Z_FAR;  velZ = 0.0f; }

    /* Death: oxygen exhausted */
    if (oxygen <= 0.0f) {
        ghost_record(posX, posY, posZ);
        deathTimer = DEATH_FRAMES;
        dead       = true;
        return;
    }

    /* Stage completion: reached the Pioneer */
    float dx = posX - PIONEER_X;
    float dy = posY - PIONEER_Y;
    float dz = posZ - PIONEER_Z;
    if (dx*dx + dy*dy + dz*dz < PIONEER_RADIUS * PIONEER_RADIUS) {
        /* TODO: transition to Stage 2 when wired */
        scene_switch(SCENE_SELECT);
        return;
    }

    /* First-person camera */
    T3DVec3 camPos    = {{posX, posY, posZ}};
    T3DVec3 camTarget = {{posX + lookX, posY + lookY, posZ + lookZ}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});
}

void scene_void_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

    /* Deep void — near-black blue. On death: dark red flash. */
    t3d_screen_clear_color(dead
        ? RGBA32(80, 10, 10, 0xFF)
        : RGBA32(2, 3, 12, 0xFF));
    t3d_screen_clear_depth();

    if (!dead) {
        /* Ghosts — flicker lighting, culled by Z proximity to player */
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
        ghost_draw(posZ, frameCount);

        /* Pioneer placeholder — warm amber point against cold void */
        uint8_t amb[4] = {15, 10, 30, 0xFF};   /* cold dark ambient */
        uint8_t dir[4] = {220, 160, 60, 0xFF};  /* warm directional */
        T3DVec3 lightDir = {{0.0f, 1.0f, 0.5f}};
        t3d_vec3_norm(&lightDir);
        t3d_light_set_ambient(amb);
        t3d_light_set_directional(0, dir, &lightDir);
        t3d_light_set_count(1);
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
        t3d_matrix_push(&pioneerMat[frameIdx]);
        draw_shape(SHAPE_OCTA);
        t3d_matrix_pop(1);
    }

    cockpit_draw_frame(posX, posY, posZ, lookX, lookY, lookZ);
    cockpit_draw_hud(oxygen);

    rdpq_detach_show();
    frameIdx = (frameIdx + 1) % 3;
}
