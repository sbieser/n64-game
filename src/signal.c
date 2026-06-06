#include <libdragon.h>
#include <math.h>
#include "signal.h"
#include "camera.h"

#define SIGNAL_CH  0

/* Beyond this distance the signal is TRULY silent — channel muted, HUD blank.
 * The void is ~28000 deep and the player spawns ~23800 units from the Pioneer,
 * so the opening minutes are dead air: stop, rotate, listen, hear nothing, try
 * another heading. Crossing into this sphere is the "blip" — the first faint
 * hum that tells you something is out there. Inside it the 1/dist curve takes
 * over and grows as you close. (stage_01_void.md: "the largest possible
 * silence", "a long signal to follow".) */
#define DETECTION_RADIUS  9000.0f

/* 1/dist falloff numerator. Tuned against DETECTION_RADIUS so the sphere edge
 * is barely audible and the signal only reads as directional up close:
 *   edge   ~9000 units → vol ≈ 0.03  (a whisper at the threshold of hearing)
 *   bearing ~2000 units → vol ≈ 0.15  (cockpit bearing console activates here)
 *   nearby   ~300 units → vol ≈ 0.97  (loud, unmistakable) */
#define SIGNAL_GAIN  300.0f

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
    /* Start silent — signal_update sets the real volume the same frame, based
     * on distance. The player spawns outside the detection radius, so this is
     * the correct opening state: nothing. */
    mixer_ch_set_vol_pan(SIGNAL_CH, 0.0f, 0.5f);
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

    /* Outside the detection sphere: dead air. Mute the channel and report no
     * bearing and zero strength, so the cockpit waveform goes flat and the
     * bearing console stays dark. This is the opening state of the stage. */
    if (dist > DETECTION_RADIUS) {
        *out_h        = 0.0f;
        *out_strength = 0.0f;
        mixer_ch_set_vol_pan(SIGNAL_CH, 0.0f, 0.5f);
        return;
    }

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

    /* Volume: 1/dist falloff (see SIGNAL_GAIN). No floor — at the sphere edge
     * the signal fades smoothly into silence. Strength mirrors volume directly
     * (0..1), driving the HUD waveform amplitude and the bearing-console gate. */
    float vol = SIGNAL_GAIN / (dist + 10.0f);
    if (vol > 1.0f) vol = 1.0f;

    *out_strength = vol;

    /* Pan drives left/right speaker balance — direction info stays in audio only */
    float pan = 0.5f + horiz * 0.45f;
    if (pan < 0.05f) pan = 0.05f;
    if (pan > 0.95f) pan = 0.95f;

    mixer_ch_set_vol_pan(SIGNAL_CH, vol, pan);
}
