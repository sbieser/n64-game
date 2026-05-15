/*
 * scene_rails.c — Forward-scrolling rails traversal
 *
 * This is the core gameplay loop prototype: the player moves forward
 * automatically through a fixed corridor of 3D space, steering with
 * the analog stick to avoid obstacles. It demonstrates the full N64
 * rendering pipeline — viewport, lighting, depth buffer, draw flags —
 * and the split between update logic and draw commands.
 *
 * THE RAIL SYSTEM
 * ───────────────
 * "On rails" means the Z position (depth into the scene) is controlled
 * by the game, not the player. railZ advances 0.4 world units per frame,
 * looping from RAIL_START (-600) back to RAIL_START when it passes RAIL_END.
 * The player can only influence lateral (X) and vertical (Y) position —
 * they can dodge left/right and up/down but cannot slow down or speed up.
 *
 * The camera sits 15 units behind railZ (camZ = railZ - 15), and looks
 * toward a point 10 units ahead of railZ. Tilting the camera target by
 * 0.6× and 0.4× of the player's lateral/vertical position gives a subtle
 * "banking" feel — the view leans into the direction of movement.
 *
 * VIEWPORT AND PROJECTION
 * ────────────────────────
 * t3d_viewport_set_projection sets the field of view (70°), near clip (10),
 * and far clip (200). Objects closer than 10 units or farther than 200 are
 * not drawn. The near plane must not be too small or Z-fighting occurs.
 * The far plane limits overdraw — obstacles beyond 200 units are simply
 * not visible, which is fine because they'd be tiny specks anyway.
 *
 * LIGHTING
 * ────────
 * libdragon's lighting system has one ambient light (flat, no direction —
 * every surface gets the same color) and up to 7 directional lights.
 * Directional lights work like the sun: they have a color and a direction
 * vector but no position, so they affect all geometry equally regardless
 * of where it is in the world.
 *
 * t3d_light_set_count(1) activates exactly one directional light.
 * The ambient fills in the shadow side so nothing goes completely black.
 *
 * Note: lightDir must be re-set every frame AFTER t3d_viewport_attach
 * because lighting is computed in view space — the direction is transformed
 * by the camera matrix, which changes every frame as the camera moves.
 *
 * DRAW FLAGS
 * ──────────
 * t3d_state_set_drawflags controls RSP/RDP pipeline state:
 *   T3D_FLAG_SHADED    — use vertex colors (not flat white)
 *   T3D_FLAG_DEPTH     — enable Z-buffer (nearer objects occlude farther ones)
 *   T3D_FLAG_CULL_BACK — skip triangles facing away from the camera (CCW = front)
 *   T3D_FLAG_NO_LIGHT  — disable normal-based lighting, use raw vertex color
 *
 * The starfield draws with NO_LIGHT so its vertex colors (white/blue/warm)
 * are used exactly as specified. Everything else draws with lighting on.
 *
 * HIT FLASH
 * ─────────
 * Collision response is purely visual — no health system yet. When
 * obstacles_check_collision() returns true, hitFlash is set to 30 frames.
 * While hitFlash > 0, the screen clears to a dark red instead of the
 * normal blue. No frame-accurate collision is needed here: even if the
 * player clips through geometry during the flash window, the game is still
 * readable because the flash communicates "something happened."
 *
 * THE UPDATE / DRAW SPLIT
 * ────────────────────────
 * scene_rails_update: runs game logic — reads input, advances the rail,
 * detects collision, computes the camera matrix. Nothing drawn here.
 *
 * scene_rails_draw: issues RSP/RDP commands only — no logic. Starts with
 * rdpq_attach (locks a framebuffer for this frame), ends with rdpq_detach_show
 * (flips the buffer to screen). Everything in between is draw commands.
 *
 * This split is important because the RSP processes commands asynchronously.
 * The CPU queues commands into a display list; the RSP executes them while
 * the CPU is already running the next frame's update. Mixing logic and draw
 * commands breaks this pipeline.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "scene.h"
#include "shapes.h"
#include "obstacles.h"
#include "starfield.h"
#include "player.h"
#include "ghost.h"
#include "phenomena.h"

#define RAIL_START      -600.0f
#define RAIL_END          50.0f
#define LATERAL_MAX       35.0f
#define VERTICAL_MAX      18.0f
#define HIT_FLASH_FRAMES  30

static T3DViewport viewport;
static float       railZ, rotAngle, lateralPos, verticalPos, camZ;
static int         hitFlash;
static bool        initialized = false;
static bool        dead        = false;
static uint32_t    frameCount  = 0;

static uint8_t ambientColor[4]     = {40,  40,  80, 0xFF};
static uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
static T3DVec3 lightDir;

/* Nebula clear color — derived from run seed each time scene is entered */
static uint8_t clearR, clearG, clearB;


void scene_rails_init(void) {
    /* New seed every run — captures hardware timer at the moment play begins */
    uint32_t seed = (uint32_t)TICKS_READ();

    if (!initialized) {
        shapes_init();
        obstacles_init();
        starfield_init(RAIL_START, RAIL_END);
        player_init();
        ghost_init();
        viewport   = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 10.0f, 200.0f);
        lightDir   = (T3DVec3){{1.0f, 1.0f, -0.5f}};
        t3d_vec3_norm(&lightDir);
        initialized = true;
    }

    obstacles_generate(seed);
    starfield_generate(seed);
    phenomena_generate(seed);

    ghost_load();

    /* Nebula tint — pull separate bits from the seed so we don't draw from
     * the same PRNG stream as the obstacle generator. Keeps the two systems
     * independent even though they share one seed. */
    clearR = 25 + ((seed >>  0) & 0x1F);  /* 25–56  — faint red shift  */
    clearG = 18 + ((seed >>  5) & 0x0F);  /* 18–33  — almost no green  */
    clearB = 70 + ((seed >> 10) & 0x3F);  /* 70–133 — dominant blue    */

    railZ       = RAIL_START;
    rotAngle    = 0.0f;
    lateralPos  = 0.0f;
    verticalPos = 0.0f;
    hitFlash    = 0;
    dead        = false;
    frameCount  = 0;
}

void scene_rails_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    frameCount++;

    if (dead) {
        if (--hitFlash <= 0) scene_switch(SCENE_SELECT);
        return;
    }

    joypad_inputs_t pad = joypad_get_inputs(JOYPAD_PORT_1);
    lateralPos  += (pad.stick_x / 85.0f) * 0.5f;
    verticalPos += (pad.stick_y / 85.0f) * 0.5f;
    phenomena_update(railZ, &lateralPos);
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

    if (obstacles_check_collision(playerZ, lateralPos, verticalPos)) {
        ghost_record(lateralPos, verticalPos, railZ);
        hitFlash = 60;
        dead     = true;
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

    float pull = phenomena_pull();
    uint8_t bgR = clearR + (pull > 0.0f ? 20 : 0);
    uint8_t bgB = clearB + (pull < 0.0f ? 25 : 0);
    t3d_screen_clear_color(hitFlash > 0
        ? RGBA32(100, 20, 20, 0xFF)
        : RGBA32(bgR, clearG, bgB, 0xFF));
    t3d_screen_clear_depth();

    t3d_light_set_ambient(ambientColor);
    t3d_light_set_directional(0, directionalColor, &lightDir);
    t3d_light_set_count(1);

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK | T3D_FLAG_NO_LIGHT);
    starfield_draw();

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);

    ghost_draw(camZ, frameCount);

    /* Restore normal lighting for player and obstacles */
    t3d_light_set_ambient(ambientColor);
    t3d_light_set_directional(0, directionalColor, &lightDir);
    t3d_light_set_count(1);
    player_draw();
    obstacles_draw(camZ);

    rdpq_detach_show();
}
