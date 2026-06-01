#pragma once
#include <stdint.h>

/*
 * dust.h — Stage 1 Layer 4: cosmic dust particles (velocity feedback).
 *
 * A small cloud of points that live in a box *around the camera*. They stream
 * past as the player thrusts and settle to a slow drift when stopped — pure
 * velocity feedback, so featureless space still feels like movement.
 *
 * HOW IT WORKS
 * ────────────
 * Each particle is stored as a camera-RELATIVE offset, not a fixed world point.
 * Each frame the offset shifts by −(camera velocity) plus a tiny intrinsic
 * drift, then wraps within the box. So when the camera moves +V, the particles
 * appear to flow −V past it (streaming); when V is zero only the slow drift
 * remains (settling). They are drawn as tiny camera-facing billboard quads at
 * cameraPos + offset, so they parallax correctly and read as genuinely near —
 * the counterpoint to the infinitely-distant star carpet.
 *
 * USAGE
 *   dust_init(seed);                       // once
 *   dust_update(velX, velY, velZ);         // each frame, camera velocity
 *   dust_draw(camX,camY,camZ, lookX,lookY,lookZ);  // each frame, in the 3D pass
 */

void dust_init(uint32_t seed);
void dust_update(float camVelX, float camVelY, float camVelZ);
void dust_draw(float camX, float camY, float camZ,
               float lookX, float lookY, float lookZ);
