#include <libdragon.h>
#include <math.h>
#include "signal.h"
#include "camera.h"

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
                   float *out_h, float *out_strength) {
    float dx   = targetX - posX;
    float dy   = targetY - posY;
    float dz   = targetZ - posZ;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);

    if (dist < 1.0f) {
        *out_h        = 0.0f;
        *out_strength = 1.0f;
        mixer_ch_set_vol_pan(SIGNAL_CH, 1.0f, 0.5f);
        return;
    }

    float sdx = dx / dist;
    float sdz = dz / dist;

    /* Project the signal direction onto the camera's right axis.
     * Horizontal bearing: -1 = hard left, 0 = dead ahead, +1 = hard right. */
    CameraBasis cb = camera_basis(lookX, lookY, lookZ);
    float horiz = sdx * cb.rightX + sdz * cb.rightZ;
    *out_h = horiz;

    /*
     * Volume: 1/dist falloff.
     *   start  ~1811 units → vol ≈ 0.16  (faint but present)
     *   nearby  ~200 units → vol ≈ 0.93  (loud)
     *   contact   ~0 units → vol = 1.0   (clamped)
     */
    float vol = 300.0f / (dist + 10.0f);
    if (vol > 1.0f)  vol = 1.0f;
    if (vol < 0.04f) vol = 0.04f;

    *out_strength = (vol - 0.04f) / 0.96f;

    /* Pan drives left/right speaker balance — direction info stays in audio only */
    float pan = 0.5f + horiz * 0.45f;
    if (pan < 0.05f) pan = 0.05f;
    if (pan > 0.95f) pan = 0.95f;

    mixer_ch_set_vol_pan(SIGNAL_CH, vol, pan);
}
