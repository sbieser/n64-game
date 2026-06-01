#pragma once

/*
 * camera.h — camera basis vectors from a look direction.
 *
 * WHY THIS EXISTS
 * ───────────────
 * Two places need the camera's "right" and "up" axes derived from the look
 * direction: the cockpit frame (to orient the 3D shell so it tracks the view)
 * and the signal system (to project the Pioneer's bearing onto left/right).
 * Both used to recompute the same cross-product math by hand; if one changed
 * its singularity guard or sign convention, they would silently disagree.
 * This module is the one definition they share.
 *
 * THE MATH
 * ────────
 * Given a unit look vector and world-up = (0, 1, 0):
 *
 *   right = normalize(look × worldUp)
 *   up    = right × look
 *
 * Because worldUp has only a Y component, the cross products simplify a lot:
 * `right` always has Y = 0 (it lies in the horizontal plane), and `up.y`
 * reduces to the horizontal length of `look`.  This is exactly the basis
 * t3d_viewport_look_at builds internally, so geometry oriented with it lines
 * up perfectly with the rendered view at any yaw/pitch.
 *
 * SINGULARITY
 * ───────────
 * When look points straight up or down, its horizontal length is zero and
 * `right` is undefined (you can spin freely about a vertical axis).  All
 * scenes clamp pitch to ±80°, keeping the horizontal length above ~0.17, but
 * the helper still guards the divide so a degenerate look can never produce
 * a NaN or a divide-by-zero FPU exception.
 *
 * The input look vector is assumed already normalized (the scenes build it
 * from yaw/pitch trig, which is unit length by construction).
 */

typedef struct {
    float rightX, rightY, rightZ;   /* camera right axis (rightY is always 0) */
    float upX,    upY,    upZ;      /* camera up axis */
} CameraBasis;

CameraBasis camera_basis(float lookX, float lookY, float lookZ);
