#include <libdragon.h>
#include <math.h>
#include "scene.h"

#define SAMPLE_RATE   22050
#define NUM_OSC           6
#define SIN_LUT_SIZE    512
#define SCOPE_LEN       512

/* Waveform types — each produces a different timbre */
typedef enum { WAVE_SINE, WAVE_SQUARE, WAVE_SAW, WAVE_TRI, WAVE_NOISE } WaveType;

typedef struct {
    WaveType wave;

    /* Frequency */
    float freq_base;        /* steady-state pitch Hz */
    float freq_env;         /* added Hz, decays toward 0 — used for pitch drops */
    float freq_env_start;   /* freq_env resets to this on each trigger */
    float freq_env_decay;   /* per-sample multiplier for freq_env (< 1.0) */

    /* Amplitude */
    float base_amp;
    float env;              /* 0..1, decays each sample */
    float env_decay;        /* per-sample multiplier (< 1.0) — set to 1.0 for drone mode */
    bool  use_env;          /* true = AD envelope triggered by LFO; false = LFO swells amp directly */

    /* LFO — controls retriggering rate (use_env) or amplitude swell (drone) */
    float lfo_rate;         /* Hz */
    float lfo_depth;        /* fraction of amplitude that LFO modulates (drone mode only) */
    float lfo_phase;        /* 0..1, current position */

    /* Noise oscillator state (xorshift32) */
    uint32_t noise_state;

    float phase;            /* oscillator phase 0..1 */
} Osc;

/* Preset config — static data loaded into Osc on preset switch */
typedef struct {
    WaveType wave;
    float freq_base, freq_env_start, freq_env_decay;
    float base_amp, env_decay;
    bool  use_env;
    float lfo_rate, lfo_depth, lfo_phase_start;
} PresetOsc;

typedef struct {
    const char  *name;
    const char  *desc;
    PresetOsc    oscs[NUM_OSC];
} Preset;

/* ---- Preset table --------------------------------------------------------
   freq_env_decay: how fast the pitch drop completes.
     0.9958 ≈ 50ms,  0.9965 ≈ 60ms,  0.9985 ≈ 150ms
   env_decay: how fast amplitude fades to zero after trigger.
     0.9950 ≈ 40ms (hi-hat),  0.9975 ≈ 80ms (snare),
     0.9990 ≈ 200ms (bloop),  1.0000 = never decays (drone)
*/
static const Preset presets[] = {

    /* 0 — SPACE DRONE: detuned sine pairs with ultra-slow LFO swells.
       Two slightly mismatched frequencies beat against each other — that
       slow "wub" is just two sine waves fighting over the same pitch. */
    { "SPACE DRONE", "sine pairs / slow LFO swell", {
        /* wave        base     env_s env_d  amp    env_d  use_env lfo_r  depth  ph   */
        { WAVE_SINE,   60.0f,   0,    1,     0.30f, 1.0f,  false,  0.07f, 0.70f, 0.0f },
        { WAVE_SINE,   60.5f,   0,    1,     0.25f, 1.0f,  false,  0.11f, 0.80f, 0.1f },
        { WAVE_SINE,  120.0f,   0,    1,     0.20f, 1.0f,  false,  0.13f, 0.60f, 0.2f },
        { WAVE_SINE,  180.0f,   0,    1,     0.15f, 1.0f,  false,  0.17f, 0.70f, 0.3f },
        { WAVE_SINE,  240.0f,   0,    1,     0.10f, 1.0f,  false,  0.19f, 0.80f, 0.4f },
        { WAVE_SINE,  360.0f,   0,    1,     0.07f, 1.0f,  false,  0.23f, 0.90f, 0.5f },
    }},

    /* 1 — CHIP TUNE: square waves in a C-major arpeggio at 6 Hz.
       Three oscillators staggered by 1/3 of a cycle so only one "speaks"
       at a time — that's what makes it sound like a melody, not a chord.
       Each note decays before the next fires, punching through cleanly. */
    { "CHIP TUNE", "square arp / AD envelope", {
        /* wave         base     env_s env_d  amp    env_d   use_env lfo_r  depth  ph    */
        { WAVE_SQUARE,  262.0f,  0,    1,     0.22f, 0.9990f,true,   6.0f,  1.0f,  0.0f  },  /* C4 */
        { WAVE_SQUARE,  330.0f,  0,    1,     0.22f, 0.9990f,true,   6.0f,  1.0f,  0.333f},  /* E4 */
        { WAVE_SQUARE,  392.0f,  0,    1,     0.18f, 0.9990f,true,   6.0f,  1.0f,  0.667f},  /* G4 */
        { WAVE_SQUARE,  131.0f,  0,    1,     0.16f, 1.0f,   false,  0.5f,  0.40f, 0.0f  },  /* C3 bass */
        { WAVE_SQUARE,  523.0f,  0,    1,     0.10f, 0.9985f,true,   3.0f,  1.0f,  0.5f  },  /* C5 beep */
        { WAVE_SQUARE,  196.0f,  0,    1,     0.12f, 0.9985f,true,   1.5f,  1.0f,  0.25f },  /* G3 pulse */
    }},

    /* 2 — BLOOP MACHINE: sine waves with pitch-drop envelopes.
       Each fires at a different prime-ish rate so they never align — the
       result is an irregular rain of retro game bloops, each one a pitch
       that slides down as it fades. Classic coin / pickup / bubble sound. */
    { "BLOOP MACHINE", "sine + pitch drop / async rates", {
        { WAVE_SINE,  200.0f, 400,0.9965f, 0.30f,0.9990f,true, 0.5f, 1.0f, 0.0f },
        { WAVE_SINE,  250.0f, 300,0.9965f, 0.25f,0.9990f,true, 0.7f, 1.0f, 0.2f },
        { WAVE_SINE,  150.0f, 500,0.9965f, 0.22f,0.9990f,true, 0.4f, 1.0f, 0.4f },
        { WAVE_SINE,  300.0f, 200,0.9965f, 0.18f,0.9990f,true, 0.9f, 1.0f, 0.6f },
        { WAVE_SINE,  180.0f, 350,0.9965f, 0.14f,0.9990f,true, 0.6f, 1.0f, 0.8f },
        { WAVE_SINE,  350.0f, 150,0.9965f, 0.10f,0.9990f,true, 1.1f, 1.0f, 0.1f },
    }},

    /* 3 — BEAT BOX: kick + snare + hi-hat at ~100 BPM.
       Kick: sine with fast pitch drop (240→40 Hz in ~50ms) = the "thump".
       Snare: noise burst with short decay = the "crack".
       Hat: noise, very short decay, double-time = the "tick".
       Snare fires at lfo_phase 0.5 so it lands between the kicks. */
    { "BEAT BOX", "sine kick + noise snare/hat", {
        { WAVE_SINE,   40.0f, 200,0.9958f, 0.55f,0.9970f,true, 1.67f,1.0f, 0.0f },  /* kick */
        { WAVE_SINE,   45.0f, 180,0.9958f, 0.45f,0.9970f,true, 1.67f,1.0f, 0.0f },  /* kick layer */
        { WAVE_NOISE,   0.0f, 0,  1,       0.35f,0.9975f,true, 1.67f,1.0f, 0.5f },  /* snare */
        { WAVE_NOISE,   0.0f, 0,  1,       0.28f,0.9975f,true, 1.67f,1.0f, 0.5f },  /* snare layer */
        { WAVE_NOISE,   0.0f, 0,  1,       0.18f,0.9950f,true, 3.34f,1.0f, 0.0f },  /* hi-hat */
        { WAVE_NOISE,   0.0f, 0,  1,       0.14f,0.9950f,true, 3.34f,1.0f, 0.5f },  /* hi-hat off-beat */
    }},
};

#define NUM_PRESETS ((int)(sizeof(presets) / sizeof(presets[0])))

static float sin_lut[SIN_LUT_SIZE];
static Osc   oscs[NUM_OSC];
static short scope_buf[SCOPE_LEN];
static int   scope_head  = 0;
static int   cur_preset  = 0;
static bool  initialized = false;

static float master_vol = 0.65f;
static float pitch_mult = 1.0f;

/* Per-preset bar colors */
static const uint8_t BAR_COLORS[NUM_OSC][3] = {
    {100, 80, 220}, {80, 120, 220},
    {60, 160, 200}, {60, 200, 160},
    {80, 220, 120}, {120, 220, 80},
};

static inline float sin_fast(float phase) {
    return sin_lut[(int)(phase * SIN_LUT_SIZE) & (SIN_LUT_SIZE - 1)];
}

static inline float wave_sample(int o) {
    float p = oscs[o].phase;
    switch (oscs[o].wave) {
        case WAVE_SINE:   return sin_fast(p);
        case WAVE_SQUARE: return p < 0.5f ? 1.0f : -1.0f;
        case WAVE_SAW:    return 2.0f * p - 1.0f;
        case WAVE_TRI:    return p < 0.5f ? (4.0f*p - 1.0f) : (3.0f - 4.0f*p);
        case WAVE_NOISE: {
            uint32_t s = oscs[o].noise_state;
            s ^= s << 13; s ^= s >> 17; s ^= s << 5;
            oscs[o].noise_state = s;
            return (float)(int32_t)s / 2147483648.0f;
        }
    }
    return 0.0f;
}

static void preset_load(int n) {
    cur_preset = n;
    for (int o = 0; o < NUM_OSC; o++) {
        const PresetOsc *p = &presets[n].oscs[o];
        oscs[o].wave           = p->wave;
        oscs[o].freq_base      = p->freq_base;
        oscs[o].freq_env_start = p->freq_env_start;
        oscs[o].freq_env_decay = p->freq_env_decay;
        oscs[o].freq_env       = 0.0f;
        oscs[o].base_amp       = p->base_amp;
        oscs[o].env            = p->use_env ? 0.0f : 1.0f;
        oscs[o].env_decay      = p->env_decay;
        oscs[o].use_env        = p->use_env;
        oscs[o].lfo_rate       = p->lfo_rate;
        oscs[o].lfo_depth      = p->lfo_depth;
        oscs[o].lfo_phase      = p->lfo_phase_start;
        oscs[o].noise_state    = 0x12345678u ^ (uint32_t)o * 0x9e3779b9u;
        oscs[o].phase          = 0.0f;
    }
}

static void synth_fill(short *buf, size_t nsamples) {
    float inv_sr = 1.0f / (float)SAMPLE_RATE;
    float mv = master_vol;
    float pm = pitch_mult;

    for (size_t i = 0; i < nsamples; i++) {
        float s = 0.0f;

        for (int o = 0; o < NUM_OSC; o++) {
            /* Advance LFO; trigger envelope on phase wrap */
            oscs[o].lfo_phase += oscs[o].lfo_rate * inv_sr;
            if (oscs[o].lfo_phase >= 1.0f) {
                oscs[o].lfo_phase -= 1.0f;
                if (oscs[o].use_env) {
                    oscs[o].env     = 1.0f;
                    oscs[o].freq_env = oscs[o].freq_env_start;
                }
            }

            /* Compute amplitude */
            float amp;
            if (oscs[o].use_env) {
                amp = oscs[o].base_amp * oscs[o].env;
                oscs[o].env *= oscs[o].env_decay;
            } else {
                float lfo_raw = 0.5f + 0.5f * sin_fast(oscs[o].lfo_phase);
                amp = oscs[o].base_amp * (1.0f - oscs[o].lfo_depth + oscs[o].lfo_depth * lfo_raw);
            }

            /* Generate sample */
            s += wave_sample(o) * amp;

            /* Advance oscillator phase */
            float freq = (oscs[o].freq_base + oscs[o].freq_env) * pm;
            oscs[o].phase += freq * inv_sr;
            if (oscs[o].phase >= 1.0f) oscs[o].phase -= 1.0f;

            /* Decay pitch envelope */
            oscs[o].freq_env *= oscs[o].freq_env_decay;
        }

        s *= mv;
        if (s >  1.0f) s =  1.0f;
        if (s < -1.0f) s = -1.0f;
        short v = (short)(s * 28000.0f);
        buf[i * 2 + 0] = v;
        buf[i * 2 + 1] = v;
        scope_buf[scope_head & (SCOPE_LEN - 1)] = v;
        scope_head++;
    }
}

void scene_synth_init(void) {
    if (!initialized) {
        for (int i = 0; i < SIN_LUT_SIZE; i++)
            sin_lut[i] = sinf(i * (2.0f * (float)M_PI) / SIN_LUT_SIZE);
        audio_init(SAMPLE_RATE, 4);
        initialized = true;
    }
    preset_load(cur_preset);
}

void scene_synth_update(void) {
    joypad_poll();
    joypad_inputs_t  pad = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    if (btn.b)       { scene_switch(SCENE_SELECT); return; }
    if (btn.d_right) preset_load((cur_preset + 1) % NUM_PRESETS);
    if (btn.d_left)  preset_load((cur_preset + NUM_PRESETS - 1) % NUM_PRESETS);

    master_vol += (pad.stick_y / 85.0f) * 0.006f;
    if (master_vol > 1.0f) master_vol = 1.0f;
    if (master_vol < 0.0f) master_vol = 0.0f;

    pitch_mult += (pad.stick_x / 85.0f) * 0.004f;
    if (pitch_mult > 2.0f) pitch_mult = 2.0f;
    if (pitch_mult < 0.5f) pitch_mult = 0.5f;

    while (audio_can_write()) {
        short *buf = audio_write_begin();
        synth_fill(buf, audio_get_buffer_length());
        audio_write_end();
    }
}

void scene_synth_draw(void) {
    rdpq_attach(display_get(), NULL);
    rdpq_clear(RGBA32(5, 5, 15, 0xFF));

    /* --- Oscilloscope -------------------------------------------------- */
    int scope_x0  = 40;
    int scope_cy  = 80;
    int scope_amp = 42;
    int scope_w   = 240;
    int head = scope_head;

    rdpq_set_mode_fill(RGBA32(25, 25, 50, 0xFF));
    rdpq_fill_rectangle(scope_x0, scope_cy, scope_x0 + scope_w, scope_cy + 1);

    rdpq_set_mode_fill(RGBA32(80, 210, 80, 0xFF));
    for (int x = 0; x < scope_w; x++) {
        int idx = (head - scope_w + x) & (SCOPE_LEN - 1);
        float v = scope_buf[idx] / 28000.0f;
        int y = scope_cy - (int)(v * scope_amp);
        rdpq_fill_rectangle(scope_x0 + x, y, scope_x0 + x + 1, y + 2);
    }

    /* --- Oscillator bars ----------------------------------------------- */
    int bar_max_h = 52;
    int bar_w     = 28;
    int bar_gap   = 12;
    int bars_span = NUM_OSC * bar_w + (NUM_OSC - 1) * bar_gap;
    int bar_x0    = (320 - bars_span) / 2;
    int bar_base  = 185;

    for (int o = 0; o < NUM_OSC; o++) {
        /* Show envelope level for AD mode, LFO swell for drone mode */
        float level = oscs[o].use_env ? oscs[o].env
                                      : (0.5f + 0.5f * sin_fast(oscs[o].lfo_phase));
        int h  = (int)(level * bar_max_h);
        if (h < 2) h = 2;
        int bx = bar_x0 + o * (bar_w + bar_gap);
        rdpq_set_mode_fill(RGBA32(BAR_COLORS[o][0], BAR_COLORS[o][1], BAR_COLORS[o][2], 0xFF));
        rdpq_fill_rectangle(bx, bar_base - h, bx + bar_w, bar_base);
        rdpq_set_mode_fill(RGBA32(BAR_COLORS[o][0]/5, BAR_COLORS[o][1]/5, BAR_COLORS[o][2]/5, 0xFF));
        rdpq_fill_rectangle(bx, bar_base - bar_max_h, bx + bar_w, bar_base - h);
    }

    /* --- HUD ----------------------------------------------------------- */
    rdpq_text_printf(NULL, 1, 8,  14, "%d/%d  %s", cur_preset + 1, NUM_PRESETS, presets[cur_preset].name);
    rdpq_text_printf(NULL, 1, 8,  26, "%s", presets[cur_preset].desc);
    rdpq_text_printf(NULL, 1, 8,  38, "VOL:%3.0f%%  PITCH:x%.2f", master_vol * 100.0f, pitch_mult);
    rdpq_text_print(NULL, 1, 8, 228, "L/R preset  Y vol  X pitch  B back");

    rdpq_detach_show();
}
