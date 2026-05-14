#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
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
static int   ghostCount;

static T3DMat4FP *cursorMat;
static T3DMat4FP *ghostMats;

static uint8_t ambientColor[4]     = {60,  60, 120, 0xFF};
static uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
static uint8_t ghostDir[4]         = {30,  40,  90, 0xFF};
static T3DVec3 lightDir;
static uint32_t frameCount = 0;

/* Two alternating ambient values — flicker makes ghosts feel unstable */
static const uint8_t ghostAmbientA[4] = {60,  80, 180, 0xFF};
static const uint8_t ghostAmbientB[4] = {40,  55, 130, 0xFF};

static void ghost_mat_rebuild(int i) {
    t3d_mat4fp_from_srt_euler(&ghostMats[i],
        (float[3]){1.0f, 1.0f, 1.0f},
        (float[3]){0.0f, 0.0f, 0.0f},
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
        ghost_mat_rebuild(ghostCount);
        ghostCount++;
        eeprom_save();
    }

    if (btn.z) {
        ghostCount = 0;
        eeprom_save();
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

    /* Ghosts: flicker ambient every other frame — makes them feel unstable */
    t3d_light_set_ambient((frameCount % 2 == 0) ? ghostAmbientA : ghostAmbientB);
    t3d_light_set_directional(0, ghostDir, &lightDir);
    t3d_light_set_count(1);
    for (int i = 0; i < ghostCount; i++) {
        t3d_matrix_push(&ghostMats[i]);
        draw_shape(SHAPE_OCTA);
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
