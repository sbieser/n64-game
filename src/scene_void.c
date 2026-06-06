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
 *
 * VISUAL LAYERS (stage_01_void.md)
 * ─────────────────────────────────
 * The void is composed of five layers. Their key distinction is what drives
 * them: the far layers respond to view ANGLE (never get closer), the near
 * layers respond to POSITION/velocity (parallax). That contrast is what makes
 * the void feel both infinite and flown-through.
 *
 *   1. Star carpet  — DONE (starcarpet.c). 2D, angle-driven, infinitely far.
 *   2. Nebula washes — DONE (nebula.c). Angle-driven, infinitely far, NOT
 *                      position-driven. Color is DIRECTIONAL — cold violet
 *                      toward the signal/Pioneer, deep blue behind, indigo
 *                      aside. Turning to face the Pioneer reveals violet; a
 *                      quiet seeking cue, never an arrow. Soft gradient blobs
 *                      that melt into the void. "A quality of the darkness."
 *   3. Debris field  — DONE (debris.c). The 3D-landmark layer, realized as a
 *                      field of rock/ice chunks instead of bare stars. True
 *                      parallax sells motion, AND their density is a navigation
 *                      gradient: thin scatter everywhere, thickening toward the
 *                      Pioneer. Following the debris inward leads to the signal.
 *   4. Cosmic dust   — DONE (dust.c). Near, position/velocity-driven; streams
 *                      past on thrust, settles at rest.
 *   5. Beacon pulse  — DONE (beacon.c). Dim, slow, irregular expanding rings
 *                      centered on the Pioneer. "Something is there," not "follow me."
 */

#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "scene.h"
#include "cockpit.h"
#include "ghost.h"
#include "shapes.h"
#include "signal.h"
#include "flight.h"
#include "nebula.h"
#include "galaxy.h"
#include "starcarpet.h"
#include "dust.h"
#include "beacon.h"
#include "debris.h"

/* Stage volume.
 *
 * The void is deliberately ENORMOUS — "the largest possible silence"
 * (stage_01_void.md). At the shared flight speed (~480 units/sec) the long
 * Z axis takes ~60 seconds to cross at full thrust, and that's only if you
 * fly straight. Wandering off-heading in search of the signal costs real
 * minutes. The Pioneer is buried deep and off to one side (see below) so the
 * player never spawns pointed at it.
 *
 * NOTE: all bounds and the Pioneer position must stay under ~32000. tiny3d's
 * model matrices are s16.16 fixed-point, so a world coordinate above 32767
 * overflows the translation term and the object jumps. 28000 leaves headroom. */
#define BOUND_X      14000.0f
#define BOUND_Y       6000.0f
#define BOUND_Z_FAR -28000.0f
#define BOUND_Z_NEAR  2000.0f

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

/* DEV KNOB — scales the oxygen drain so we can fly the whole enormous void and
 * tune the search without dying before first contact. The intended stage is
 * brutal (you may not make it on the first visit — that's the design), but the
 * 90s tank is unplayable while the space is this big and the signal this faint.
 * Set back to 1.0f to restore the real tank. */
#define OXY_DEV_SCALE   0.25f

/* Pioneer beacon ghost — placeholder until humanoid model is ready.
 * Buried deep in −Z and pushed off-axis in +X so the spawn heading (straight
 * down −Z) does NOT point at it. Spawn-to-Pioneer distance (~23800) is well
 * outside the signal's detection radius, so the player starts in dead silence. */
#define PIONEER_X     9000.0f
#define PIONEER_Y     1500.0f
#define PIONEER_Z   -22000.0f
#define PIONEER_RADIUS  100.0f    /* proximity for stage completion */
#define PIONEER_SCALE   50.0f     /* placeholder scale — a silhouette on arrival */

/* Frames of red-screen death flash before returning to select */
#define DEATH_FRAMES    90

static T3DViewport  viewport;
static T3DMat4FP   *pioneerMat;   /* [3] triple-buffered — position never changes */
static Flight       flight;       /* shared first-person kinematics */
static float        oxygen;
static int          deathTimer;
static bool         dead;
static uint32_t     frameCount;
static int          frameIdx;
static float        signalH, signalStrength;
static bool         initialized = false;

/* Stage volume + shared flight feel. Bounds are wider than the Star Field. */
static const FlightConfig flightCfg = {
    .boundX = BOUND_X, .boundY = BOUND_Y,
    .boundZNear = BOUND_Z_NEAR, .boundZFar = BOUND_Z_FAR,
    .thrust = THRUST, .drag = DRAG, .maxSpeed = MAX_SPEED,
    .yawRate = YAW_RATE, .pitchRate = PITCH_RATE, .pitchMax = PITCH_MAX,
};

void scene_void_init(void) {
    if (!initialized) {
        shapes_init();
        ghost_init();
        signal_init();
        /* Background star carpet (Layer 1) + cosmic dust (Layer 4). Fixed seeds
         * for now — wire the run seed here once it reaches Stage 1. */
        galaxy_init(0x6A1A89u);
        starcarpet_init(0x51A5F1E1u);
        dust_init(0x0D05721Du);
        beacon_init(0xBEAC04u, PIONEER_X, PIONEER_Y, PIONEER_Z);
        /* Debris field — fixed seed; density gradient peaks at the Pioneer. */
        debris_init(0xD3B215u, PIONEER_X, PIONEER_Y, PIONEER_Z);

        pioneerMat = malloc_uncached(sizeof(T3DMat4FP) * 3);
        float scale[3] = {PIONEER_SCALE, PIONEER_SCALE, PIONEER_SCALE};
        float rot[3]   = {0.0f, 0.0f, 0.0f};
        float pos[3]   = {PIONEER_X, PIONEER_Y, PIONEER_Z};
        for (int i = 0; i < 3; i++)
            t3d_mat4fp_from_srt_euler(&pioneerMat[i], scale, rot, pos);

        viewport = t3d_viewport_create();
        /* Far clip is intentionally MUCH shorter than the stage is deep
         * (8000 vs 28000). The void is too big to render end-to-end, and we
         * don't want to: the 2D star backdrop is screen-space (no depth, always
         * visible), and the only 3D objects — Pioneer, ghosts, dust, beacon —
         * only matter when the player is near them. A shorter far plane also
         * keeps the 16-bit depth buffer precise where it counts: up close. */
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.0f), 2.0f, 8000.0f);

        initialized = true;
    }

    ghost_load();

    flight_reset(&flight);
    signalH = 0.0f; signalStrength = 0.0f;
    signal_play();
    oxygen     = 1.0f;
    deathTimer = 0;
    dead       = false;
    frameCount = 0;
    frameIdx   = 0;
}

void scene_void_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.b) { signal_stop(); scene_switch(SCENE_SELECT); return; }

    frameCount++;

    /* After death, count down the flash then return to select */
    if (dead) {
        if (--deathTimer <= 0) { signal_stop(); scene_switch(SCENE_SELECT); }
        return;
    }

    joypad_inputs_t  pad  = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);

    /* Shared flight kinematics. Oxygen drains only while thrust is applied. */
    bool thrusting = flight_update(&flight, &flightCfg,
                                   pad.stick_x, pad.stick_y, held.z, held.r);
    if (thrusting) {
        oxygen -= OXY_DRAIN_RATE * OXY_DEV_SCALE;
        if (oxygen < 0.0f) oxygen = 0.0f;
    }

    /* Cosmic dust streams past based on the camera's velocity this frame. */
    dust_update(flight.velX, flight.velY, flight.velZ);

    /* Advance the Pioneer's beacon ring timers. */
    beacon_update();

    /* Death: oxygen exhausted */
    if (oxygen <= 0.0f) {
        ghost_record(flight.posX, flight.posY, flight.posZ);
        deathTimer = DEATH_FRAMES;
        dead       = true;
        return;
    }

    /* Stage completion: reached the Pioneer */
    float dx = flight.posX - PIONEER_X;
    float dy = flight.posY - PIONEER_Y;
    float dz = flight.posZ - PIONEER_Z;
    if (dx*dx + dy*dy + dz*dz < PIONEER_RADIUS * PIONEER_RADIUS) {
        /* TODO: transition to Stage 2 when wired */
        signal_stop();
        scene_switch(SCENE_SELECT);
        return;
    }

    /* First-person camera */
    T3DVec3 camPos    = {{flight.posX, flight.posY, flight.posZ}};
    T3DVec3 camTarget = {{flight.posX + flight.lookX,
                          flight.posY + flight.lookY,
                          flight.posZ + flight.lookZ}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0, 1, 0}});

    signal_update(flight.posX, flight.posY, flight.posZ,
                  flight.lookX, flight.lookY, flight.lookZ,
                  PIONEER_X, PIONEER_Y, PIONEER_Z,
                  &signalH, &signalStrength);
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
        /* Far backdrop — three 2D layers locked to the same view-angle scroll
         * (skyproj.h), composited bottom-up so they move as one sky:
         *   carpet (stars) → nebula (additive color glow) → galaxy (points).
         * The nebula glows over the stars without erasing them; the galaxy
         * sits on top. All leave the RDP in fill/standard 2D modes, so restore
         * standard mode before the 3D scene. */
        starcarpet_draw(flight.yaw, flight.pitch);
        nebula_draw(flight.yaw, flight.pitch);
        galaxy_draw(flight.yaw, flight.pitch);
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

        /* Ghosts — flicker lighting, culled by Z proximity to player */
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);
        ghost_draw(flight.posZ, frameCount);

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

        /* Layer 3 — debris field. Rock/ice chunks scattered through the void,
         * thickening toward the Pioneer. Sets its own cold lighting; only the
         * pieces within render range are emitted. */
        debris_draw(flight.posX, flight.posY, flight.posZ);

        /* Layer 5 — the Pioneer's expanding beacon ring(s). */
        beacon_draw();

        /* Layer 4 — cosmic dust, the nearest 3D layer (drawn over the scene,
         * under the cockpit). */
        dust_draw(flight.posX, flight.posY, flight.posZ,
                  flight.lookX, flight.lookY, flight.lookZ);
    }

    cockpit_draw_frame(flight.posX, flight.posY, flight.posZ,
                       flight.lookX, flight.lookY, flight.lookZ);
    cockpit_draw_hud(oxygen, signalH, signalStrength);

    rdpq_detach_show();
    frameIdx = (frameIdx + 1) % 3;
}
