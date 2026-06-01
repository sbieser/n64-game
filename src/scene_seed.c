/*
 * scene_seed.c — Seeded run generation visualizer
 *
 * This scene demonstrates and tests the PRNG system that will drive
 * procedural content in the actual game. Every run of The Colossus
 * derives from a single uint32_t seed — obstacle positions, star
 * density, debris clustering all come from sequential draws on one
 * fast pseudo-random number generator initialized from that seed.
 * Same seed = identical run, every time.
 *
 * WHY ONE SEED FOR THE WHOLE RUN
 * ───────────────────────────────
 * A single seed makes runs deterministic and, in principle, replayable.
 * It also makes saves extremely compact: storing the seed + run index
 * is enough to reconstruct the entire run layout. Ghost positions don't
 * need to store the full obstacle field — just the seed they died on.
 *
 * WHERE THE SEED COMES FROM
 * ──────────────────────────
 * TICKS_READ() reads the MIPS R4300i's CP0 Count register — a hardware
 * counter that increments every CPU cycle (93.75 MHz / 2 = 46.875 MHz).
 * It starts at 0 on power-on and counts continuously. The exact value
 * at the moment the player starts a run depends on how long the title
 * screen was displayed, menu navigation time, etc. This provides enough
 * entropy to make each run feel different even if the player boots and
 * immediately starts.
 *
 * THE XORSHIFT32 PRNG
 * ────────────────────
 * Three XOR-and-shift operations on a 32-bit state:
 *
 *   state ^= state << 13
 *   state ^= state >> 17
 *   state ^= state << 5
 *
 * Each call advances the state and returns it. The specific shifts
 * (13, 17, 5) are chosen to produce a full-period sequence — the
 * generator visits all 2^32 - 1 non-zero values before repeating.
 * It is fast (3 operations), cheap (no multiplication), and produces
 * good statistical distribution for game use. Not cryptographic, but
 * we don't need that — we need cheap and good-looking.
 *
 * One critical property: the output is chaotic. Seed 1000 and seed 1001
 * produce completely different sequences. Press D-pad up/down to see
 * this — adjacent seeds give unrecognisably different layouts.
 *
 * DERIVING MULTIPLE VALUES FROM ONE SEED
 * ────────────────────────────────────────
 * To generate N obstacles and M star density values from one seed,
 * we reset the PRNG state to the seed and draw sequentially:
 *
 *   rng_state = seed
 *   for each obstacle: draw 4 values (z, x, y, type)
 *   for each segment:  draw 1 value (star density)
 *
 * The order is fixed and documented. Any system that needs data from
 * the seed must draw its values in the agreed order, or use a separate
 * sub-seed (seed XOR domain_constant) so streams don't interfere.
 *
 * THE VISUALIZATION
 * ──────────────────
 * Three displays show what the current seed produces:
 *
 * LATERAL STRIP — a bird's-eye view of the rail corridor. The rail
 * runs left to right (Z axis). Each dot is an obstacle; its horizontal
 * position shows where along the rail it appears, its vertical position
 * shows lateral offset (X axis). Color encodes obstacle type.
 *
 * STAR DENSITY — the rail divided into 14 segments. Bar height shows
 * relative star density in that region. Dense regions feel chaotic;
 * sparse regions feel vast and lonely — matching Acts 1 and 2.
 *
 * PRNG SAMPLE — the first 8 raw uint32_t values from this seed,
 * displayed as hex. These are the actual numbers before we map them
 * to positions or densities. Notice how different seeds produce
 * completely unrelated sequences.
 */

#include <libdragon.h>
#include "scene.h"
#include "rng.h"

#define NUM_OBSTACLES  12
#define NUM_SEGMENTS   14
#define RAIL_START   -600.0f
#define RAIL_END       50.0f
#define LATERAL_MAX    35.0f
#define VERTICAL_MAX   18.0f

/* Strip display geometry */
#define STRIP_X    16
#define STRIP_W   288
#define STRIP_LAT_Y   68   /* centre Y of lateral strip */
#define STRIP_LAT_H   28   /* half-height of lateral strip */

typedef struct { float z, x; int type; } ObSpec;

static uint32_t seed;
static uint32_t run_count = 0;
static ObSpec   obs[NUM_OBSTACLES];
static float    star_density[NUM_SEGMENTS];
static uint32_t sample_vals[8];

/* Obstacle type colors — matches shape types in the rails scene */
static const uint32_t TYPE_COLORS[5] = {
    0xFF4444FF,  /* cube     — red    */
    0x44FFFFFF,  /* tetra    — cyan   */
    0x4444FFFF,  /* octa     — blue   */
    0xFFFF44FF,  /* pyramid  — yellow */
    0xFF44FFFF,  /* prism    — magenta*/
};

static void generate(void) {
    Rng rng;

    /* Capture first 8 raw PRNG values for the sample display */
    rng_seed(&rng, seed);
    for (int i = 0; i < 8; i++) sample_vals[i] = rng_next(&rng);

    /* Re-seed and generate obstacle layout — 4 draws per obstacle */
    rng_seed(&rng, seed);
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        float rail_len = RAIL_END - RAIL_START;
        obs[i].z    = RAIL_START + rng_unit(&rng) * rail_len;
        obs[i].x    = ((int)(rng_next(&rng) % 1000) - 500) / 500.0f * LATERAL_MAX;
        /* y not visualised in the lateral strip but consumed from stream */
        rng_next(&rng); /* vertical position */
        obs[i].type = (int)rng_range(&rng, 5);
    }

    /* Star density — continue same stream after obstacles */
    for (int i = 0; i < NUM_SEGMENTS; i++)
        star_density[i] = (rng_next(&rng) % 1000) / 1000.0f;
}

void scene_seed_init(void) {
    /* Seed from hardware cycle counter — different every boot */
    seed = (uint32_t)TICKS_READ();
    run_count = 0;
    generate();
}

void scene_seed_update(void) {
    joypad_poll();
    joypad_buttons_t btn  = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);

    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    if (btn.a) {
        seed = (uint32_t)TICKS_READ();
        run_count++;
        generate();
    }

    /* D-pad lets you nudge the seed to see how chaotic adjacent values are */
    if (btn.d_up   || held.d_up)   { seed++; generate(); }
    if (btn.d_down || held.d_down) { seed--; generate(); }
}

void scene_seed_draw(void) {
    rdpq_attach(display_get(), NULL);
    rdpq_clear(RGBA32(5, 5, 15, 0xFF));

    /* --- Title and seed value ------------------------------------------ */
    rdpq_text_print(NULL,  1, 8, 14, "SEED LAB");
    rdpq_text_printf(NULL, 1, 8, 26, "SEED: 0x%08lX   run #%lu",
        (unsigned long)seed, (unsigned long)run_count);

    /* --- Lateral rail strip -------------------------------------------- */
    rdpq_text_print(NULL, 1, STRIP_X, 46, "LATERAL VIEW (top-down)");

    /* Background */
    rdpq_set_mode_fill(RGBA32(18, 18, 36, 0xFF));
    rdpq_fill_rectangle(STRIP_X, STRIP_LAT_Y - STRIP_LAT_H,
                        STRIP_X + STRIP_W, STRIP_LAT_Y + STRIP_LAT_H);

    /* Centre line */
    rdpq_set_mode_fill(RGBA32(40, 40, 80, 0xFF));
    rdpq_fill_rectangle(STRIP_X, STRIP_LAT_Y - 1, STRIP_X + STRIP_W, STRIP_LAT_Y + 1);

    /* Lateral boundary lines */
    rdpq_set_mode_fill(RGBA32(60, 40, 40, 0xFF));
    rdpq_fill_rectangle(STRIP_X, STRIP_LAT_Y - STRIP_LAT_H,
                        STRIP_X + STRIP_W, STRIP_LAT_Y - STRIP_LAT_H + 1);
    rdpq_fill_rectangle(STRIP_X, STRIP_LAT_Y + STRIP_LAT_H - 1,
                        STRIP_X + STRIP_W, STRIP_LAT_Y + STRIP_LAT_H);

    /* Obstacle dots */
    float rail_len = RAIL_END - RAIL_START;
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        int sx = STRIP_X + (int)((obs[i].z - RAIL_START) / rail_len * STRIP_W);
        int sy = STRIP_LAT_Y - (int)(obs[i].x / LATERAL_MAX * (STRIP_LAT_H - 2));
        rdpq_set_mode_fill(RGBA32(
            (TYPE_COLORS[obs[i].type] >> 24) & 0xFF,
            (TYPE_COLORS[obs[i].type] >> 16) & 0xFF,
            (TYPE_COLORS[obs[i].type] >>  8) & 0xFF,
            0xFF));
        rdpq_fill_rectangle(sx - 2, sy - 2, sx + 3, sy + 3);
    }

    /* Rail start/end markers */
    rdpq_set_mode_fill(RGBA32(80, 80, 80, 0xFF));
    rdpq_fill_rectangle(STRIP_X, STRIP_LAT_Y - STRIP_LAT_H, STRIP_X + 1, STRIP_LAT_Y + STRIP_LAT_H);
    rdpq_fill_rectangle(STRIP_X + STRIP_W - 1, STRIP_LAT_Y - STRIP_LAT_H, STRIP_X + STRIP_W, STRIP_LAT_Y + STRIP_LAT_H);

    /* --- Star density bars --------------------------------------------- */
    rdpq_text_print(NULL, 1, STRIP_X, 104, "STAR DENSITY");

    int seg_w = STRIP_W / NUM_SEGMENTS;
    int bar_base_y = 136;
    int bar_max_h  = 22;
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        int bx = STRIP_X + i * seg_w;
        int bh = (int)(star_density[i] * bar_max_h);
        if (bh < 2) bh = 2;
        /* Dim trough */
        rdpq_set_mode_fill(RGBA32(15, 15, 35, 0xFF));
        rdpq_fill_rectangle(bx, bar_base_y - bar_max_h, bx + seg_w - 1, bar_base_y - bh);
        /* Bright bar */
        uint8_t bright = (uint8_t)(40 + star_density[i] * 180);
        rdpq_set_mode_fill(RGBA32(bright / 3, bright / 3, bright, 0xFF));
        rdpq_fill_rectangle(bx, bar_base_y - bh, bx + seg_w - 1, bar_base_y);
    }

    /* --- PRNG sample values -------------------------------------------- */
    rdpq_text_print(NULL, 1, STRIP_X, 150, "PRNG SAMPLE (first 8 draws):");
    rdpq_text_printf(NULL, 1, STRIP_X, 162, "%08lX %08lX %08lX %08lX",
        (unsigned long)sample_vals[0], (unsigned long)sample_vals[1],
        (unsigned long)sample_vals[2], (unsigned long)sample_vals[3]);
    rdpq_text_printf(NULL, 1, STRIP_X, 174, "%08lX %08lX %08lX %08lX",
        (unsigned long)sample_vals[4], (unsigned long)sample_vals[5],
        (unsigned long)sample_vals[6], (unsigned long)sample_vals[7]);

    /* --- Obstacle type legend ------------------------------------------ */
    rdpq_text_print(NULL, 1, STRIP_X, 192, "TYPE:");
    const char *names[5] = {"CUBE","TETRA","OCTA","PYRAM","PRISM"};
    int lx = STRIP_X + 40;
    for (int i = 0; i < 5; i++) {
        rdpq_set_mode_fill(RGBA32(
            (TYPE_COLORS[i] >> 24) & 0xFF,
            (TYPE_COLORS[i] >> 16) & 0xFF,
            (TYPE_COLORS[i] >>  8) & 0xFF, 0xFF));
        rdpq_fill_rectangle(lx, 186, lx + 6, 196);
        rdpq_text_print(NULL, 1, lx + 8, 196, names[i]);
        lx += 52;
    }

    /* --- Controls ------------------------------------------------------ */
    rdpq_text_print(NULL, 1, 8, 216, "A:new seed   UP/DOWN:nudge seed");
    rdpq_text_print(NULL, 1, 8, 228, "B:back");

    rdpq_detach_show();
}
