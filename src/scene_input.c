#include <libdragon.h>
#include "scene.h"

void scene_input_init(void) {
    /* font id=1 is loaded and registered by scene_select (the boot scene) */
}

void scene_input_update(void) {
    joypad_poll();
    for (int p = 0; p < 4; p++) {
        joypad_buttons_t btn = joypad_get_buttons_pressed(p);
        if (btn.start) { scene_switch(SCENE_SELECT); return; }
    }
}

void scene_input_draw(void) {
    rdpq_attach(display_get(), NULL);
    rdpq_clear(RGBA32(10, 10, 20, 255));

    rdpq_text_print(NULL, 1,  8,  14, "INPUT DEBUG  (START=back)");

    for (int p = 0; p < 4; p++) {
        int y = 36 + p * 50;

        bool connected = joypad_is_connected(p);
        rdpq_text_printf(NULL, 1, 8, y, "PORT %d: %s", p + 1,
                         connected ? "CONNECTED" : "---");

        if (connected) {
            joypad_inputs_t  inp  = joypad_get_inputs(p);
            joypad_buttons_t held = joypad_get_buttons_held(p);

            rdpq_text_printf(NULL, 1, 8, y + 14,
                "A:%d B:%d Z:%d St:%d L:%d R:%d",
                held.a, held.b, held.z, held.start, held.l, held.r);

            rdpq_text_printf(NULL, 1, 8, y + 26,
                "U:%d D:%d L:%d R:%d  X:%d Y:%d",
                held.d_up, held.d_down, held.d_left, held.d_right,
                (int)inp.stick_x, (int)inp.stick_y);
        }
    }

    rdpq_detach_show();
}
