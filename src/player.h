#pragma once

/*
 * player.h — the rails-stage player ship ("The Freighter").
 *
 * A loaded T3D model positioned each frame from the rail coordinates. Used by
 * the third-person rails demo; the real game stages are first-person (cockpit)
 * and do not draw a ship.  See player.c for the scale/orientation rationale.
 *
 * LIFECYCLE
 *   player_init();                               // once — load the model
 *   player_update(lateralPos, verticalPos, railZ); // each frame — position it
 *   player_draw();                               // each frame — draw it
 */

/* Load the ship model. Call once at startup. */
void player_init(void);

/* Position the ship from the current rail coordinates. */
void player_update(float lateralPos, float verticalPos, float railZ);

/* Draw the ship at its current transform. */
void player_draw(void);
