#pragma once
#include <t3d/t3d.h>
#include "shapes.h"

#define NUM_OBSTACLES 12

typedef struct {
    float worldZ, x, y, hitRadius;
    int   shape;
    float rotSpeedX, rotSpeedY, rotPhase;
} Obstacle;

extern const Obstacle obstacles[NUM_OBSTACLES];

void obstacles_init(void);
void obstacles_update(float rotAngle);
int  obstacles_check_collision(float camZ, float lateralPos, float verticalPos);
void obstacles_draw(void);
