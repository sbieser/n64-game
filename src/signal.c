#include <libdragon.h>
#include <math.h>
#include "signal.h"

#define SIGNAL_CH  0

static wav64_t wav;
static bool    loaded = false;

void signal_init(void) {
    if (loaded) return;
    wav64_open(&wav, "rom:/signal.wav64");
    wav64_set_loop(&wav, true);
    loaded = true;
}

void signal_play(void) {
    wav64_play(&wav, SIGNAL_CH);
    mixer_ch_set_vol_pan(SIGNAL_CH, 0.04f, 0.5f);
}

void signal_stop(void) {
    mixer_ch_stop(SIGNAL_CH);
}

void signal_update(float posX,    float posY,    float posZ,
                   float lookX,   float lookY,   float lookZ,
                   float targetX, float targetY, float targetZ,
                   float *out_h, float *out_v, float *out_strength) {
    float dx   = targetX - posX;
    float dy   = targetY - posY;
    float dz   = targetZ - posZ;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);

    if (dist < 1.0f) {
        *out_h        = 0.0f;
        *out_v        = 0.0f;
        *out_strength = 1.0f;
        mixer_ch_set_vol_pan(SIGNAL_CH, 1.0f, 0.5f);
        return;
    }

    float sdx = dx / dist;
    float sdy = dy / dist;
    float sdz = dz / dist;

    /*
     * Camera basis — identical construction to cockpit_draw_frame.
     * side  = normalize(look × worldUp) = (-lookZ, 0, lookX) / sideLen
     * up    = side × look  (unit vector; uy simplifies to sideLen)
     */
    float sideLen = sqrtf(lookZ*lookZ + lookX*lookX);
    if (sideLen < 0.0001f) sideLen = 0.0001f;
    float sx = -lookZ / sideLen;   /* camera-right X */
    float sz =  lookX / sideLen;   /* camera-right Z */
    float ux = -sz * lookY;        /* camera-up    X */
    float uy =  sideLen;           /* camera-up    Y  (= sz*lookX - sx*lookZ) */
    float uz =  sx * lookY;        /* camera-up    Z */

    /* Dot products give signed [-1, +1] direction components */
    *out_h = sdx * sx + sdz * sz;              /* horizontal */
    *out_v = sdx * ux + sdy * uy + sdz * uz;  /* vertical   */

    /*
     * Volume: 1/dist falloff.  Reference distances to Pioneer (200, 0, -1800):
     *   start  ~1811 units → vol ≈ 0.16  (faint but present)
     *   nearby  ~200 units → vol ≈ 0.93  (loud)
     *   contact   ~0 units → vol = 1.0   (full, clamped)
     */
    float vol = 300.0f / (dist + 10.0f);
    if (vol > 1.0f)  vol = 1.0f;
    if (vol < 0.04f) vol = 0.04f;

    *out_strength = (vol - 0.04f) / 0.96f;    /* 0..1 for cockpit dot size */

    /* Pan: 0 = full left, 0.5 = center, 1 = full right */
    float pan = 0.5f + (*out_h) * 0.45f;
    if (pan < 0.05f) pan = 0.05f;
    if (pan > 0.95f) pan = 0.95f;

    mixer_ch_set_vol_pan(SIGNAL_CH, vol, pan);
}
