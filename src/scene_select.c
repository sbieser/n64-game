#include <libdragon.h>
#include "scene.h"

#define TILE_W    80
#define TILE_H    60
#define COLS       3
#define START_X   20
#define START_Y   50
#define COL_STEP 100
#define ROW_STEP  90
#define BORDER     2

static int          cursor = 0;
static rdpq_font_t *font   = NULL;

static color_t unpack(uint32_t c) {
    return RGBA32((c>>24)&0xFF, (c>>16)&0xFF, (c>>8)&0xFF, c&0xFF);
}

void scene_select_init(void) {
    if (!font) {
        font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
        rdpq_text_register_font(1, font);
    }
    cursor = 0;
}

void scene_select_update(void) {
    joypad_poll();
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    int num = scene_count() - 1;
    if (num <= 0) return;

    if (btn.d_right && cursor + 1 < num)    cursor++;
    if (btn.d_left  && cursor > 0)          cursor--;
    if (btn.d_down  && cursor + COLS < num) cursor += COLS;
    if (btn.d_up    && cursor - COLS >= 0)  cursor -= COLS;
    if (btn.a)                              scene_switch(cursor + 1);
}

void scene_select_draw(void) {
    rdpq_attach(display_get(), NULL);
    rdpq_clear(RGBA32(10, 10, 20, 255));

    int num = scene_count() - 1;

    /* Tile fills */
    for (int i = 0; i < num; i++) {
        const Scene *s = scene_get(i + 1);
        int tx = START_X + (i % COLS) * COL_STEP;
        int ty = START_Y + (i / COLS) * ROW_STEP;
        rdpq_set_mode_fill(unpack(s->color));
        rdpq_fill_rectangle(tx, ty, tx + TILE_W, ty + TILE_H);
    }

    /* Cursor border — white outline around selected tile */
    {
        int tx = START_X + (cursor % COLS) * COL_STEP;
        int ty = START_Y + (cursor / COLS) * ROW_STEP;
        rdpq_set_mode_fill(RGBA32(255, 255, 255, 255));
        rdpq_fill_rectangle(tx - BORDER, ty - BORDER, tx + TILE_W + BORDER, ty);
        rdpq_fill_rectangle(tx - BORDER, ty + TILE_H, tx + TILE_W + BORDER, ty + TILE_H + BORDER);
        rdpq_fill_rectangle(tx - BORDER, ty,          tx,                   ty + TILE_H);
        rdpq_fill_rectangle(tx + TILE_W, ty,          tx + TILE_W + BORDER, ty + TILE_H);
    }

    /* Text labels below each tile */
    for (int i = 0; i < num; i++) {
        const Scene *s = scene_get(i + 1);
        int tx = START_X + (i % COLS) * COL_STEP;
        int ty = START_Y + (i / COLS) * ROW_STEP;
        rdpq_text_print(NULL, 1, tx + 4, ty + TILE_H + 12, s->name);
    }

    rdpq_text_print(NULL, 1, 8,  20, "DEMO SELECT");
    rdpq_text_print(NULL, 1, 8, 228, "D-PAD move   A enter   B back");

    rdpq_detach_show();
}
