/*
 * main.c — boot sequence and the master game loop.
 *
 * The boot order matters: each subsystem depends on the ones before it.
 *   debug_*      — USB / IS-Viewer logging, harmless if no hardware listening
 *   dfs_init     — mounts the ROM filesystem so "rom:/..." paths resolve
 *   display_init — 3 framebuffers (triple buffering), 320×240, 16-bit color
 *   rdpq_init    — the RDP command queue (2D fills, blits, the cockpit HUD)
 *   t3d_init     — tiny3d: uploads the RSP microcode for 3D transforms
 *   joypad_init  — controller polling
 *   audio_init   — opens the audio DAC at 22.05 kHz with 4 back buffers
 *   mixer_init   — the RSP mixer: the single audio path. ALL sound goes
 *                  through it (the signal beacon, the synth demo) so nothing
 *                  fights over the AI buffers.
 *
 * 22.05 kHz is the native rate of both the signal WAV and the synth, so the
 * mixer outputs them without resampling.
 *
 * The loop is deliberately tiny: scene_tick() runs the current scene's
 * update() + draw(), then mixer_try_play() tops up the audio output buffers
 * (pulling fresh samples from every playing mixer channel). Audio must be
 * serviced every frame or it stutters, so it lives here at the top level
 * rather than inside any one scene.
 */

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
    audio_init(22050, 4);
    mixer_init(4);

    scene_init();

    for (;;) {
        scene_tick();
        mixer_try_play();
    }

    t3d_destroy();
    return 0;
}
