#pragma once
#include <t3d/t3d.h>
#include "shapes.h"

/*
 * obstacles.h — the rails-stage obstacle field.
 *
 * A fixed set of tumbling debris shapes positioned along the rail corridor.
 * Their entire layout is derived from the run seed (see rng.h): same seed →
 * same field, every time.  The player weaves through them; contact is a hazard.
 *
 * DETERMINISM CONTRACT
 *   obstacles_generate() draws from the seed in a FIXED order (per obstacle:
 *   worldZ, x, y, shape, rotSpeedX, rotSpeedY, rotPhase — 7 draws).  Changing
 *   that order changes every existing run, so treat it as frozen.
 *
 * LIFECYCLE
 *   obstacles_init();                 // once — allocate matrices
 *   obstacles_generate(seed);         // on stage entry — build the field
 *   obstacles_update(rotAngle, camZ); // each frame — rebuild visible matrices
 *   obstacles_check_collision(...);   // each frame — test the player figure
 *   obstacles_draw(camZ);             // each frame — draw visible obstacles
 */

#define NUM_OBSTACLES 12

typedef struct {
    float worldZ;       /* position along the rail (Z) */
    float x, y;         /* lateral / vertical offset within the corridor */
    float hitRadius;    /* collision radius (XY plane) */
    int   shape;        /* a SHAPE_* constant */
    float rotSpeedX;    /* tumble rate about X */
    float rotSpeedY;    /* tumble rate about Y */
    float rotPhase;     /* per-obstacle phase offset so they don't spin in sync */
} Obstacle;

extern Obstacle obstacles[NUM_OBSTACLES];

/* Allocate the obstacle model-matrix block. Call once at startup. */
void obstacles_init(void);

/* Build the obstacle field deterministically from the run seed. */
void obstacles_generate(uint32_t seed);

/* Rebuild model matrices for obstacles within draw distance of camZ.
 * rotAngle is the global tumble clock (each obstacle adds its own phase). */
void obstacles_update(float rotAngle, float camZ);

/* Return 1 if the player figure overlaps any obstacle, 0 otherwise. */
int  obstacles_check_collision(float playerZ, float lateralPos, float verticalPos);

/* Draw all obstacles within draw distance of camZ. */
void obstacles_draw(float camZ);
