/*
 * scene_select.c — the demo/stage picker grid (SCENE_SELECT, index 0).
 *
 * A paged grid of tiles, one per scene in the registry except Select itself.
 * The D-pad moves the cursor, A enters the highlighted scene, L/R page through
 * when there are more scenes than fit on one 3×2 page.
 *
 * It reads the scene table through scene_count() / scene_get() rather than
 * knowing any scene directly, so adding a scene to scenes[] makes it appear
 * here automatically.  The "+1" throughout skips index 0 (Select itself) so
 * the menu never lists a tile that re-enters the menu.
 */

#include <libdragon.h>
#include <string.h>
#include "scene.h"

#define TILE_W         80
#define TILE_H         60
#define COLS            3
#define ROWS            2
#define TILES_PER_PAGE (COLS * ROWS)
#define START_X        20
#define START_Y        50
#define COL_STEP      100
#define ROW_STEP       90
#define BORDER          2

static int cursor = 0;
static int page   = 0;

static color_t unpack(uint32_t c) {
    return RGBA32((c>>24)&0xFF, (c>>16)&0xFF, (c>>8)&0xFF, c&0xFF);
}

void scene_select_init(void) {
    cursor = 0;
    page   = 0;
}

void scene_select_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    int num       = scene_count() - 1;
    if (num <= 0) return;
    int num_pages  = (num + TILES_PER_PAGE - 1) / TILES_PER_PAGE;
    int page_start = page * TILES_PER_PAGE;
    int page_tiles = num - page_start;
    if (page_tiles > TILES_PER_PAGE) page_tiles = TILES_PER_PAGE;

    if (btn.l && page > 0)               { page--; cursor = 0; return; }
    if (btn.r && page < num_pages - 1)   { page++; cursor = 0; return; }

    if (btn.d_right && cursor + 1 < page_tiles)    cursor++;
    if (btn.d_left  && cursor > 0)                 cursor--;
    if (btn.d_down  && cursor + COLS < page_tiles) cursor += COLS;
    if (btn.d_up    && cursor - COLS >= 0)         cursor -= COLS;
    if (btn.a)                                     scene_switch(page_start + cursor + 1);
}

void scene_select_draw(void) {
    rdpq_attach(display_get(), NULL);
    rdpq_clear(RGBA32(10, 10, 20, 255));

    int num        = scene_count() - 1;
    int num_pages  = (num + TILES_PER_PAGE - 1) / TILES_PER_PAGE;
    int page_start = page * TILES_PER_PAGE;
    int page_tiles = num - page_start;
    if (page_tiles > TILES_PER_PAGE) page_tiles = TILES_PER_PAGE;

    /* Tile fills */
    for (int i = 0; i < page_tiles; i++) {
        const Scene *s = scene_get(page_start + i + 1);
        int tx = START_X + (i % COLS) * COL_STEP;
        int ty = START_Y + (i / COLS) * ROW_STEP;
        rdpq_set_mode_fill(unpack(s->color));
        rdpq_fill_rectangle(tx, ty, tx + TILE_W, ty + TILE_H);
    }

    /* Cursor border */
    {
        int tx = START_X + (cursor % COLS) * COL_STEP;
        int ty = START_Y + (cursor / COLS) * ROW_STEP;
        rdpq_set_mode_fill(RGBA32(255, 255, 255, 255));
        rdpq_fill_rectangle(tx - BORDER, ty - BORDER, tx + TILE_W + BORDER, ty);
        rdpq_fill_rectangle(tx - BORDER, ty + TILE_H, tx + TILE_W + BORDER, ty + TILE_H + BORDER);
        rdpq_fill_rectangle(tx - BORDER, ty,          tx,                   ty + TILE_H);
        rdpq_fill_rectangle(tx + TILE_W, ty,          tx + TILE_W + BORDER, ty + TILE_H);
    }

    /* Text labels centred inside each tile */
    for (int i = 0; i < page_tiles; i++) {
        const Scene *s = scene_get(page_start + i + 1);
        int tx = START_X + (i % COLS) * COL_STEP;
        int ty = START_Y + (i / COLS) * ROW_STEP;
        int name_w = (int)strlen(s->name) * 8;
        int lx = tx + (TILE_W - name_w) / 2;
        if (lx < tx + 2) lx = tx + 2;
        rdpq_text_print(NULL, 1, lx, ty + TILE_H / 2 + 4, s->name);
    }

    rdpq_text_printf(NULL, 1, 8, 20, "DEMO SELECT   %d / %d", page + 1, num_pages);
    rdpq_text_print(NULL, 1, 8, 228, "D-PAD move   A enter   L/R page");

    rdpq_detach_show();
}
