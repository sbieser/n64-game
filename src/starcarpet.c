#include <libdragon.h>
#include "starcarpet.h"
#include "rng.h"
#include "skyproj.h"

#define SCREEN_W   320
#define SCREEN_H   240
#define NUM_STARS  220     /* dense enough to feel overwhelming, cheap to fill */

/* Scroll rate comes from skyproj.h (SKY_SCROLL_X/Y) so the carpet, nebula, and
 * galaxies all move in lockstep. Turn right (yaw+) pushes stars left; look up
 * (pitch+) pushes them down. */

/* Star "types": hue + brightness + size. Drawn one group at a time so the fill
 * color is set once per type, not once per star. Weighted toward dim cold white
 * with rare blue-white and faint warm points (per stage_01_void.md). */
#define NUM_TYPES  5
static const uint8_t TYPE_RGB[NUM_TYPES][3] = {
    {110, 110, 120},   /* 0 — dim white     */
    {170, 170, 180},   /* 1 — mid white     */
    {235, 235, 245},   /* 2 — bright white  */
    {150, 175, 235},   /* 3 — blue-white    */
    {180, 150, 120},   /* 4 — faint warm    */
};
static const uint8_t TYPE_SIZE[NUM_TYPES] = { 1, 1, 2, 1, 1 };

/* Map a 0..99 roll to a type, weighted: 55% dim, 25% mid, 10% bright,
 * 7% blue-white, 3% warm. */
static int type_for(uint32_t roll) {
    if (roll < 55) return 0;
    if (roll < 80) return 1;
    if (roll < 90) return 2;
    if (roll < 97) return 3;
    return 4;
}

typedef struct { int16_t bx, by; uint8_t type; } Star;
static Star stars[NUM_STARS];

void starcarpet_init(uint32_t seed) {
    Rng rng;
    rng_seed(&rng, seed);
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].bx   = (int16_t)rng_range(&rng, SCREEN_W);
        stars[i].by   = (int16_t)rng_range(&rng, SCREEN_H);
        stars[i].type = (uint8_t)type_for(rng_range(&rng, 100));
    }
}

/* Positive modulo so scrolling left/up wraps cleanly. */
static inline int wrap(int v, int m) {
    v %= m;
    if (v < 0) v += m;
    return v;
}

void starcarpet_draw(float yaw, float pitch) {
    int offX = (int)(-yaw  * SKY_SCROLL_X);
    int offY = (int)( pitch * SKY_SCROLL_Y);

    for (int t = 0; t < NUM_TYPES; t++) {
        rdpq_set_mode_fill(RGBA32(TYPE_RGB[t][0], TYPE_RGB[t][1], TYPE_RGB[t][2], 255));
        int sz = TYPE_SIZE[t];
        for (int i = 0; i < NUM_STARS; i++) {
            if (stars[i].type != t) continue;
            int x = wrap(stars[i].bx + offX, SCREEN_W);
            int y = wrap(stars[i].by + offY, SCREEN_H);
            rdpq_fill_rectangle(x, y, x + sz, y + sz);
        }
    }
}
