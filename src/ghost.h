#pragma once
#include <stdint.h>

/*
 * ghost.h — death-record persistence and frozen-wreck rendering.
 *
 * When the player dies, their position is written to cartridge save memory.
 * On later runs those positions load back as still, frozen figures — the
 * player's own death history made visible, and the environmental storytelling
 * of "other seekers who came before".
 *
 * Capacity is capped (currently 8); when full, the oldest record is evicted.
 * Storage is EEPROM today; the design calls for SRAM (32 KB) to allow denser
 * ghost fields later.  See mechanics/ghosts.md for the full mechanic.
 *
 * LIFECYCLE
 *   ghost_init();                 // once at startup — loads the model, probes EEPROM
 *   ghost_load();                 // on stage entry — read saved records into memory
 *   ghost_record(x, y, z);        // on death — append + persist (evicts oldest if full)
 *   ghost_draw(camZ, frameCount); // each frame — draw records near the camera
 */

/* Load the ghost model and detect save hardware. Call once at startup. */
void ghost_init(void);

/* Read persisted ghost records from EEPROM into memory. Call on stage entry. */
void ghost_load(void);

/* Append a ghost at (x,y,z) and persist. Evicts the oldest if at capacity. */
void ghost_record(float x, float y, float z);

/* Draw all ghosts within draw distance of camZ. `frame` drives the every-other-
 * frame ambient flicker that makes wrecks feel unstable (SF64 technique). */
void ghost_draw(float camZ, uint32_t frame);

/* Number of ghosts currently loaded. */
int  ghost_count(void);
