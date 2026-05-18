/*
 * scene_ghost.c — 3D ghost placement with EEPROM persistence
 *
 * This scene is a proof-of-concept for the game's core ghost mechanic:
 * positions planted here persist across power cycles, then render as dim
 * frozen figures. In the final game these positions come from player deaths
 * along the rail — here you place them manually to explore the feel.
 *
 * CONTROLS
 * ────────
 * Analog stick moves the cursor left/right and up/down in world space.
 * L/R buttons move it forward/backward (Z depth). A plants a ghost at the
 * cursor's current position. Z clears all ghosts. B returns to the menu.
 *
 * EEPROM SAVE SYSTEM
 * ──────────────────
 * The N64 cartridge has a small EEPROM chip — either 4Kbit (512 bytes) or
 * 16Kbit depending on the game. We request 4Kbit in the Makefile via
 * N64_ED64ROMCONFIGFLAGS = -w eeprom4k, which writes a header that Ares
 * and flashcarts read to automatically configure save hardware.
 *
 * The save layout (GhostSave struct, 104 bytes, packed to avoid padding):
 *   [0..3]   magic number 0x47484F53 ("GHOS") — validates the save
 *   [4..7]   ghost count
 *   [8..39]  x positions (8 floats × 4 bytes)
 *   [40..71] y positions
 *   [72..103] z positions
 *
 * On boot, eeprom_load() reads those bytes and checks the magic number.
 * If it matches, ghost positions are restored and their matrices rebuilt.
 * If it doesn't (first boot, or corrupted save), we start fresh.
 *
 * Why __attribute__((packed))?
 * The C compiler may insert padding bytes between struct members to keep
 * them aligned on natural boundaries. packed forces zero padding so the
 * struct's byte layout exactly matches what we write/read. This matters
 * because eeprom_write_bytes/read_bytes work on raw bytes — any padding
 * would silently corrupt the data.
 *
 * WHY UNCACHED MEMORY FOR MATRICES?
 * ──────────────────────────────────
 * The RSP (Reality Signal Processor) DMA's matrices directly from RDRAM.
 * The CPU has an 8KB cache — if the matrix lives in cached memory, the RSP
 * might read stale data that hasn't been flushed from the CPU cache yet.
 * malloc_uncached() allocates from a region that bypasses the CPU cache
 * entirely, so the RSP always sees the value the CPU just wrote.
 *
 * GHOST RENDERING
 * ────────────────
 * Each ghost has a pre-built T3DMat4FP (fixed-point 4×4 matrix) stored in
 * ghostMats[i]. When a ghost is planted, ghost_mat_rebuild() computes the
 * transform once using t3d_mat4fp_from_srt_euler (scale/rotation/translation)
 * and stores it. Every frame we just push that pre-built matrix and draw.
 * Matrices are only rebuilt when a ghost is planted — not every frame.
 *
 * The cursor matrix IS rebuilt every frame (in update) because the cursor
 * moves continuously with the analog stick.
 *
 * VISUAL DIFFERENTIATION
 * ──────────────────────
 * The cursor and ghosts use the same octahedron mesh (SHAPE_OCTA) but
 * different lighting. The cursor gets full warm ambient + directional so
 * it reads as the "live" object. Ghosts use a dim cool-blue ambient with
 * a faint directional — they feel cold and still.
 *
 * FLICKER EFFECT
 * ──────────────
 * Ghost ambient lighting alternates between two values every frame
 * (ghostAmbientA and ghostAmbientB). At 60fps the flicker is fast enough
 * to read as a shimmer rather than a flash. This makes ghosts feel unstable
 * and barely present — a technique taken from Star Fox 64's use of
 * gGameFrameCount % 2 for visual shimmer on effects.
 *
 * The cursor is never flickered — the visual contrast between the stable
 * cursor and the unstable ghosts reinforces which one is "alive."
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "scene.h"
#include "shapes.h"

#define MAX_GHOSTS   8
#define EEPROM_MAGIC 0x47484F53u  /* "GHOS" */

/* Packed EEPROM layout: magic + count + up to 8 XYZ positions = 104 bytes */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t count;
    float    x[MAX_GHOSTS];
    float    y[MAX_GHOSTS];
    float    z[MAX_GHOSTS];
} GhostSave;

static T3DViewport viewport;
static bool        initialized = false;
static bool        eeprom_ok   = false;

static float curX, curY, curZ;
static float ghostX[MAX_GHOSTS];
static float ghostY[MAX_GHOSTS];
static float ghostZ[MAX_GHOSTS];
static float ghostRot[MAX_GHOSTS];   /* accumulated tumble angle per ghost */
static int   ghostCount;

static T3DMat4FP *cursorMat;
static T3DMat4FP *ghostMats;
static T3DModel  *wreckModel;

static uint8_t ambientColor[4]     = {60,  60, 120, 0xFF};
static uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
static uint8_t ghostDir[4]         = {80, 100, 200, 0xFF};
static T3DVec3 lightDir;
static uint32_t frameCount = 0;

/* Two alternating ambient values — flicker makes ghosts feel unstable */
static const uint8_t ghostAmbientA[4] = {100, 110, 220, 0xFF};
static const uint8_t ghostAmbientB[4] = { 60,  70, 160, 0xFF};

static void ghost_mat_rebuild(int i) {
    float r = ghostRot[i];
    t3d_mat4fp_from_srt_euler(&ghostMats[i],
        (float[3]){0.05f, 0.05f, 0.05f},
        (float[3]){r * 0.31f, r, r * 0.53f},   /* multi-axis tumble */
        (float[3]){ghostX[i], ghostY[i], ghostZ[i]}
    );
}

static void eeprom_save(void) {
    if (!eeprom_ok) return;
    GhostSave data = {.magic = EEPROM_MAGIC, .count = (uint32_t)ghostCount};
    for (int i = 0; i < ghostCount; i++) {
        data.x[i] = ghostX[i];
        data.y[i] = ghostY[i];
        data.z[i] = ghostZ[i];
    }
    eeprom_write_bytes((uint8_t *)&data, 0, sizeof(GhostSave));
}

static void eeprom_load(void) {
    if (!eeprom_ok) return;
    GhostSave data;
    eeprom_read_bytes((uint8_t *)&data, 0, sizeof(GhostSave));
    if (data.magic != EEPROM_MAGIC) return;
    if (data.count > MAX_GHOSTS) data.count = MAX_GHOSTS;
    ghostCount = (int)data.count;
    for (int i = 0; i < ghostCount; i++) {
        ghostX[i] = data.x[i];
        ghostY[i] = data.y[i];
        ghostZ[i] = data.z[i];
        ghostRot[i] = 0.0f;
        ghost_mat_rebuild(i);
    }
}

void scene_ghost_init(void) {
    if (!initialized) {
        shapes_init();
        cursorMat = malloc_uncached(sizeof(T3DMat4FP));
        ghostMats = malloc_uncached(sizeof(T3DMat4FP) * MAX_GHOSTS);
        viewport  = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 1.0f, 120.0f);
        lightDir  = (T3DVec3){{1.0f, 1.0f, -0.5f}};
        t3d_vec3_norm(&lightDir);
        eeprom_ok  = eeprom_present() != EEPROM_NONE;
        ghostCount = 0;
        wreckModel = t3d_model_load("rom:/ghost_wreck.t3dm");
        eeprom_load();
        initialized = true;
    }
    curX = 0.0f;
    curY = 0.0f;
    curZ = 0.0f;
}

void scene_ghost_update(void) {
    joypad_poll();
    joypad_inputs_t  pad  = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t btn  = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);

    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    curX += (pad.stick_x / 85.0f) * 0.4f;
    curY += (pad.stick_y / 85.0f) * 0.4f;
    if (held.l) curZ += 0.2f;
    if (held.r) curZ -= 0.2f;

    if (curX >  20.0f) curX =  20.0f;
    if (curX < -20.0f) curX = -20.0f;
    if (curY >  14.0f) curY =  14.0f;
    if (curY < -14.0f) curY = -14.0f;
    if (curZ >  15.0f) curZ =  15.0f;
    if (curZ < -20.0f) curZ = -20.0f;

    if (btn.a && ghostCount < MAX_GHOSTS) {
        ghostX[ghostCount] = curX;
        ghostY[ghostCount] = curY;
        ghostZ[ghostCount] = curZ;
        ghostRot[ghostCount] = 0.0f;
        ghostCount++;
        eeprom_save();
    }

    if (btn.z) {
        ghostCount = 0;
        eeprom_save();
    }

    /* Advance tumble rotation per ghost — each spins at a slightly different rate */
    for (int i = 0; i < ghostCount; i++) {
        ghostRot[i] += 0.008f + (i % 4) * 0.003f;
        ghost_mat_rebuild(i);
    }

    t3d_mat4fp_from_srt_euler(cursorMat,
        (float[3]){1.0f, 1.0f, 1.0f},
        (float[3]){0.0f, 0.0f, 0.0f},
        (float[3]){curX, curY, curZ}
    );

    T3DVec3 camPos    = {{0.0f, 8.0f, -50.0f}};
    T3DVec3 camTarget = {{0.0f, 0.0f,   0.0f}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
    frameCount++;
}

void scene_ghost_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

    t3d_screen_clear_color(RGBA32(5, 5, 15, 0xFF));
    t3d_screen_clear_depth();

    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);

    /* Wrecks: cold dim ambient, tumbling slowly */
    t3d_light_set_ambient((frameCount % 2 == 0) ? ghostAmbientA : ghostAmbientB);
    t3d_light_set_directional(0, ghostDir, &lightDir);
    t3d_light_set_count(1);
    for (int i = 0; i < ghostCount; i++) {
        t3d_matrix_push(&ghostMats[i]);
        t3d_model_draw(wreckModel);
        t3d_matrix_pop(1);
    }

    /* Cursor: full lighting so it reads clearly against the dim ghosts */
    t3d_light_set_ambient(ambientColor);
    t3d_light_set_directional(0, directionalColor, &lightDir);
    t3d_light_set_count(1);
    t3d_matrix_push(cursorMat);
    draw_shape(SHAPE_OCTA);
    t3d_matrix_pop(1);

    /* HUD */
    rdpq_text_print(NULL, 1, 8, 14, "GHOST SCENE");
    rdpq_text_printf(NULL, 1, 8, 26, "Ghosts: %d/%d  %s",
        ghostCount, MAX_GHOSTS, eeprom_ok ? "EEPROM OK" : "NO EEPROM");
    rdpq_text_print(NULL, 1, 8, 216, "STICK move  L/R depth  A plant");
    rdpq_text_print(NULL, 1, 8, 228, "Z clear all  B back");

    rdpq_detach_show();
}
