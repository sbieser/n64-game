#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "ghost.h"

#define MAX_GHOSTS    8
#define GHOST_MAGIC   0x52454C53u  /* "RELS" */
#define EEPROM_OFFSET 128          /* ghost demo occupies 0..103 */
#define DRAW_DIST     160.0f

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t count;
    float    x[MAX_GHOSTS];
    float    y[MAX_GHOSTS];
    float    z[MAX_GHOSTS];
} GhostSave;

static int        count     = 0;
static float      gx[MAX_GHOSTS];
static float      gy[MAX_GHOSTS];
static float      gz[MAX_GHOSTS];
static T3DMat4FP *mats;
static bool       eeprom_ok = false;
static T3DVec3    lightDir;
static T3DModel  *ghostModel;

static const uint8_t ambA[4]   = {40,  60, 160, 0xFF};
static const uint8_t ambB[4]   = {25,  40, 110, 0xFF};
static const uint8_t dirCol[4] = {20,  30,  80, 0xFF};

static void rebuild_mat(int i) {
    t3d_mat4fp_from_srt_euler(&mats[i],
        (float[3]){0.05f, 0.05f, 0.05f},
        (float[3]){0.0f, 3.14159265f, 0.0f},
        (float[3]){gx[i], gy[i], gz[i]}
    );
}

static void ghost_save(void) {
    if (!eeprom_ok) return;
    GhostSave data = {.magic = GHOST_MAGIC, .count = (uint32_t)count};
    for (int i = 0; i < count; i++) {
        data.x[i] = gx[i]; data.y[i] = gy[i]; data.z[i] = gz[i];
    }
    eeprom_write_bytes((uint8_t *)&data, EEPROM_OFFSET, sizeof(GhostSave));
}

void ghost_init(void) {
    mats       = malloc_uncached(sizeof(T3DMat4FP) * MAX_GHOSTS);
    eeprom_ok  = eeprom_present() != EEPROM_NONE;
    lightDir   = (T3DVec3){{-0.5f, 1.0f, -0.5f}};
    t3d_vec3_norm(&lightDir);
    ghostModel = t3d_model_load("rom:/ghost_reacher.t3dm");
}

void ghost_load(void) {
    count = 0;
    if (!eeprom_ok) return;
    GhostSave data;
    eeprom_read_bytes((uint8_t *)&data, EEPROM_OFFSET, sizeof(GhostSave));
    if (data.magic != GHOST_MAGIC) return;
    if (data.count > MAX_GHOSTS) data.count = MAX_GHOSTS;
    count = (int)data.count;
    for (int i = 0; i < count; i++) {
        gx[i] = data.x[i]; gy[i] = data.y[i]; gz[i] = data.z[i];
        rebuild_mat(i);
    }
}

void ghost_record(float x, float y, float z) {
    if (count < MAX_GHOSTS) {
        int idx = count++;
        gx[idx] = x; gy[idx] = y; gz[idx] = z;
        rebuild_mat(idx);
    } else {
        /* Shift out oldest, add new at end */
        for (int i = 0; i < MAX_GHOSTS - 1; i++) {
            gx[i] = gx[i+1]; gy[i] = gy[i+1]; gz[i] = gz[i+1];
            rebuild_mat(i);
        }
        int last = MAX_GHOSTS - 1;
        gx[last] = x; gy[last] = y; gz[last] = z;
        rebuild_mat(last);
    }
    ghost_save();
}

void ghost_draw(float camZ, uint32_t frame) {
    t3d_light_set_ambient((frame % 2 == 0) ? ambA : ambB);
    t3d_light_set_directional(0, dirCol, &lightDir);
    t3d_light_set_count(1);
    for (int i = 0; i < count; i++) {
        float dz = gz[i] - camZ;
        if (dz < -20.0f || dz > DRAW_DIST) continue;
        t3d_matrix_push(&mats[i]);
        t3d_model_draw(ghostModel);
        t3d_matrix_pop(1);
    }
}

int ghost_count(void) { return count; }
