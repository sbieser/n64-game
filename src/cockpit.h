#pragma once

/*
 * cockpit.h — 3D frame model + 2D HUD overlay.
 *
 * cockpit_init()      — load cockpit.t3dm, call once at startup.
 * cockpit_draw_frame(posX, posY, posZ, lookX, lookY, lookZ)
 *                     — draw the 3D cockpit frame locked to the camera.
 *                       lookX/Y/Z must be a unit look vector matching the
 *                       viewport's look direction (same as passed to look_at).
 *                       Call inside t3d_frame_start() / rdpq_detach_show(),
 *                       after the world scene, before the HUD.
 * cockpit_draw_hud(oxygen_level)
 *                     — draw 2D rdpq oxygen indicators and signal screen.
 *                       Call after cockpit_draw_frame(), before rdpq_detach_show().
 *
 * oxygen_level: 0.0 (empty) → 1.0 (full).
 *   8-square indicator cluster: cyan at full, red at ≤ 25%, dark at 0.
 */

void cockpit_init(void);
void cockpit_draw_frame(float posX, float posY, float posZ,
                        float lookX, float lookY, float lookZ);
void cockpit_draw_hud(float oxygen_level, float signal_h, float signal_strength);
