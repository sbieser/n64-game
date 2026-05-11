#pragma once
#include <t3d/t3d.h>

#define SHAPE_CUBE    0
#define SHAPE_TETRA   1
#define SHAPE_OCTA    2
#define SHAPE_PYRAMID 3
#define SHAPE_PRISM   4

void shapes_init(void);
void draw_shape(int shape);
