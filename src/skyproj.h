#pragma once
#include <math.h>

/*
 * skyproj.h — shared "2D skybox" projection for the far backdrop layers.
 *
 * The star carpet, nebula washes, and galaxies are all 2D and scroll with view
 * ANGLE at the SAME rate, so they lock together as one infinitely-distant sky
 * and never parallax with position (the SF64 trick). Keeping the scroll
 * constants and the projection in one place is what guarantees the layers stay
 * locked — if these ever differed per layer, the formations would drift across
 * the stars (which is exactly the bug this replaced).
 *
 * Rates are pixels of screen movement per radian of view rotation, matched to
 * the camera field of view (~227 px/rad horizontal, ~212 px/rad vertical).
 */

#define SKY_SCROLL_X 227.0f
#define SKY_SCROLL_Y 212.0f
#define SKY_CX       160.0f
#define SKY_CY       120.0f

/*
 * Project a fixed sky direction to a screen position, given the camera angle.
 *   az  — azimuth, radians. 0 = straight ahead, + = to the right.
 *   el  — elevation, radians. 0 = level, + = up.
 * The object is centered on screen when the camera faces it (yaw==az,
 * pitch==el). Off-screen results mean "behind you / out of view" — cull them.
 */
static inline void sky_project(float az, float el, float yaw, float pitch,
                               float *sx, float *sy) {
    float dyaw = az - yaw;
    while (dyaw >  3.14159265f) dyaw -= 6.28318531f;
    while (dyaw < -3.14159265f) dyaw += 6.28318531f;
    *sx = SKY_CX + dyaw * SKY_SCROLL_X;
    *sy = SKY_CY + (pitch - el) * SKY_SCROLL_Y;
}
