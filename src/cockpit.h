#pragma once

/*
 * cockpit.h — 2D rdpq overlay drawn after the 3D scene each frame.
 *
 * The cockpit frame (pillars, top bar, dashboard divider) and instruments
 * (indicator cluster, signal screen, gloved hand) are fixed-position quads
 * at 320×240. The windshield area (x 18–302, y 8–150) has no geometry —
 * the 3D scene shows through.
 *
 * Call cockpit_draw() after all t3d draw calls and before rdpq_detach_show().
 * The overlay switches the RDP to fill mode; it does not restore the prior mode.
 *
 * Layout (screen coords, y increases downward):
 *
 *   ┌──────────────────────────────────────┐  y=0
 *   │ [top bar 8px]                        │
 *   ├─────────────────────────────────────┤  y=8
 *   │P│                              │P│  │
 *   │I│   VOID — 3D scene shows here │I│  │
 *   │L│                              │L│  │
 *   │L│                              │L│  │
 *   ├─────────────────────────────────────┤  y=150
 *   │  [mid frame strip 6px]              │
 *   ├─────────────────────────────────────┤  y=156
 *   │  [indicator cluster]  [screen] [hand]│
 *   │           DASHBOARD                 │
 *   └──────────────────────────────────────┘  y=240
 *
 *   Pillars: x 0–18 (left), x 302–320 (right)
 */

void cockpit_draw(float oxygen_level);

/*
 * oxygen_level: 0.0 (empty) to 1.0 (full).
 * Drives the 8-square indicator cluster:
 *   - All cyan at 1.0
 *   - Squares go dark right-to-left as level drops
 *   - Remaining squares turn red at <= 0.25
 */
