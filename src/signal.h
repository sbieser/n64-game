#pragma once

/*
 * signal.h — Pioneer beacon audio and direction.
 *
 * signal_init()      — load rom:/signal.wav64 once at startup.
 * signal_play()      — start the looping beacon (call on stage enter).
 * signal_stop()      — stop playback (call before scene_switch).
 * signal_update()    — call once per frame to set mixer pan/volume and
 *                      compute the direction values for the cockpit screen.
 *
 *   out_h:        -1 (signal hard left) .. +1 (signal hard right)
 *   out_v:        -1 (signal below)     .. +1 (signal above)
 *   out_strength: 0 (far / inaudible)   .. 1 (close / loud)
 *
 * lookX/Y/Z must be a unit vector — same one passed to cockpit_draw_frame.
 */

void signal_init(void);
void signal_play(void);
void signal_stop(void);
void signal_update(float posX,    float posY,    float posZ,
                   float lookX,   float lookY,   float lookZ,
                   float targetX, float targetY, float targetZ,
                   float *out_h, float *out_strength);
