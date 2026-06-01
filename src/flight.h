#pragma once
#include <stdbool.h>

/*
 * flight.h — first-person free-roam flight kinematics.
 *
 * WHY THIS EXISTS
 * ───────────────
 * The seeking stages (Void, Star Field, and more to come) all share the same
 * movement model: the stick steers heading, the Z/R buttons thrust along the
 * look direction, velocity persists with drag so the ship coasts to a stop.
 * That "stop, rotate, listen, thrust" loop is the core of the game, so the
 * kinematics must feel identical everywhere.  Two scenes used to carry a
 * line-for-line copy of it; this module is the single implementation they share.
 *
 * WHAT IT OWNS vs WHAT THE SCENE OWNS
 * ───────────────────────────────────
 * flight owns only the kinematics: heading, velocity, position, and the derived
 * look vector.  Everything stage-specific — oxygen, death, the Pioneer, the
 * camera attach, the cockpit — stays in the scene.  The scene calls
 * flight_update() once per frame, then reads f.pos* and f.look* to drive its
 * camera and HUD.
 *
 * The flight model:
 *   heading   yaw += stickX·yawRate,  pitch += stickY·pitchRate (clamped ±pitchMax)
 *   look      from yaw/pitch trig (unit vector; (0,0,-1) at yaw=pitch=0)
 *   thrust    Z adds look·thrust to velocity, R subtracts it
 *   drag      velocity ×= drag each frame  → coasting stop
 *   speed cap clamped by vector magnitude so terminal speed is heading-independent
 *   integrate position += velocity
 *   bounds    soft clamp to the stage volume; hitting an edge zeroes that axis
 *
 * CONFIG
 * ──────
 * The feel constants (thrust, drag, rates) are shared across stages, but the
 * stage volume differs, so they live in a FlightConfig the scene passes in.
 */

typedef struct {
    float posX, posY, posZ;     /* world position */
    float velX, velY, velZ;     /* world velocity */
    float yaw, pitch;           /* heading, radians */
    float lookX, lookY, lookZ;  /* unit look vector derived from yaw/pitch */
} Flight;

typedef struct {
    float boundX, boundY;           /* soft bounds: |x| ≤ boundX, |y| ≤ boundY */
    float boundZNear, boundZFar;    /* boundZFar is negative (into the scene) */
    float thrust;                   /* velocity added per frame of thrust */
    float drag;                     /* velocity retained each frame (0..1) */
    float maxSpeed;                 /* terminal speed (vector magnitude) */
    float yawRate, pitchRate;       /* radians per frame at full stick deflection */
    float pitchMax;                 /* pitch clamp, radians (avoids gimbal flip) */
} FlightConfig;

/* Reset to origin, zero velocity, heading forward (look = (0,0,-1)). */
void flight_reset(Flight *f);

/* Advance one frame.  stickX/stickY are raw joypad axis values (±~85);
 * thrustForward/Backward are the Z/R button held states.
 * Returns true if any thrust was applied this frame (for oxygen accounting). */
bool flight_update(Flight *f, const FlightConfig *cfg,
                   float stickX, float stickY,
                   bool thrustForward, bool thrustBackward);
