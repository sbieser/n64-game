#pragma once
#include <stdint.h>

/*
 * starcarpet.h — Stage 1 Layer 1: the 2D background star carpet.
 *
 * THE SF64 INSIGHT
 * ────────────────
 * These stars have no Z. They are 2D screen-space dots whose scroll is driven
 * by the camera's *angle* (yaw/pitch), not its position. So no matter how far
 * the player flies, the carpet never gets closer — it behaves like an infinitely
 * distant sky. That is what makes the void feel boundless (the Star Fox 64
 * technique; see DESIGN.md "Starfield — 2D Skybox, Not 3D Space").
 *
 * Depth then comes from contrast: this carpet scrolls at the camera's angular
 * rate (as if at infinity), while the foreground 3D stars and dust (later
 * layers) parallax faster because they're genuinely near. The carpet is the
 * still backdrop; the 3D layers sell the motion.
 *
 * Drawn as 2D rdpq fills, so it must be rendered AFTER the screen clear but
 * BEFORE the 3D scene (which then draws on top). It leaves the RDP in fill
 * mode — the caller switches back to standard mode before 3D draws.
 *
 * USAGE
 *   starcarpet_init(seed);              // once — seed the star positions/colors
 *   ...clear color + depth...
 *   starcarpet_draw(flight.yaw, pitch); // each frame, before the 3D scene
 *   rdpq_set_mode_standard();           // restore standard mode for 3D
 */

/* Seed the fixed 2D star positions, colors, and sizes. Call once. */
void starcarpet_init(uint32_t seed);

/* Draw the carpet, scrolled by the current view angle (radians). */
void starcarpet_draw(float yaw, float pitch);
