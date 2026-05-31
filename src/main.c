#include <libdragon.h>
#include <t3d/t3d.h>
#include "scene.h"

int main(void) {
    debug_init_isviewer();
    debug_init_usblog();

    dfs_init(DFS_DEFAULT_LOCATION);
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
    t3d_init((T3DInitParams){});
    joypad_init();
    audio_init(44100, 4);
    mixer_init(4);

    scene_init();

    for (;;) {
        scene_tick();
        mixer_try_play();
    }

    t3d_destroy();
    return 0;
}
