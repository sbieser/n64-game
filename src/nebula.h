#pragma once

/*
 * nebula.h — Stage 1 Layer 2: nebula color washes (2D, angle-locked).
 *
 * Two-to-three enormous, dim color regions giving parts of the void distinct
 * identity: cold violet toward the signal/Pioneer, deep blue behind, indigo to
 * one side. "Not objects — a quality of the darkness." (stage_01_void.md)
 *
 * APPROACH
 * ────────
 * 2D screen-space, using the SAME angle scroll as the star carpet (skyproj.h),
 * so the washes lock to the stars and move as one sky. Each wash is a soft
 * gradient blob (a shaded triangle fan, bright center → transparent rim) drawn
 * with ADDITIVE blending ON TOP of the carpet — so it glows color over the void
 * and the stars sparkle right through it, never erased. Each wash sits at a
 * fixed sky direction, so turning to face the Pioneer brings the violet into
 * view: a quiet directional cue, never an arrow.
 *
 * USAGE
 *   nebula_draw(yaw, pitch);   // each frame, after the carpet, before the galaxy
 */

void nebula_draw(float yaw, float pitch);
