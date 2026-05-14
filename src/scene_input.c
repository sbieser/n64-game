/*
 * scene_input.c — Live joypad state debugger
 *
 * Displays real-time button and stick state for all four controller ports.
 * Useful for verifying hardware and understanding the joypad API.
 *
 * HOW THE JOYPAD API WORKS
 * ─────────────────────────
 * joypad_poll() samples all ports and stores the result internally.
 * After polling, three query functions give different views of the state:
 *
 *   joypad_get_inputs(port)         — raw axis values (stick_x, stick_y as
 *                                     signed 8-bit integers, ±85 at full tilt)
 *
 *   joypad_get_buttons_held(port)   — buttons currently held down this frame
 *
 *   joypad_get_buttons_pressed(port)— buttons that transitioned from up→down
 *                                     this frame (one-shot, not held)
 *
 * joypad_is_connected(port) checks whether a controller is physically present.
 * We display this status but always show data regardless — some USB adapters
 * report as disconnected even when they work. Gating display on connectivity
 * would hide valid input, which is worse than showing zeros for empty ports.
 *
 * FONT SHARING
 * ────────────
 * rdpq_text_register_font must be called exactly once per font ID.
 * Calling it twice on the same ID crashes with an assertion. The builtin
 * debug font is loaded in scene_init() (scene.c) and registered as ID 1,
 * so all scenes share it without each needing to load their own copy.
 * That's why scene_input_init() is empty — nothing to set up.
 *
 * NO Z-BUFFER
 * ───────────
 * rdpq_attach(display_get(), NULL) passes NULL for the depth buffer.
 * This scene is pure 2D text — there is no 3D geometry, so depth testing
 * is unnecessary and omitting it saves a small amount of RDP overhead.
 */

#include <libdragon.h>
#include "scene.h"

void scene_input_init(void) {
    /* font id=1 is loaded once globally by scene_init() in scene.c */
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

        joypad_inputs_t  inp  = joypad_get_inputs(p);
        joypad_buttons_t held = joypad_get_buttons_held(p);

        rdpq_text_printf(NULL, 1, 8, y, "PORT %d: %s", p + 1,
                         joypad_is_connected(p) ? "CONNECTED" : "---");

        rdpq_text_printf(NULL, 1, 8, y + 14,
            "A:%d B:%d Z:%d St:%d L:%d R:%d",
            held.a, held.b, held.z, held.start, held.l, held.r);

        rdpq_text_printf(NULL, 1, 8, y + 26,
            "U:%d D:%d L:%d R:%d  X:%d Y:%d",
            held.d_up, held.d_down, held.d_left, held.d_right,
            (int)inp.stick_x, (int)inp.stick_y);
    }

    rdpq_detach_show();
}
