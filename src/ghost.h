#pragma once
#include <stdint.h>

void ghost_init(void);
void ghost_load(void);
void ghost_record(float x, float y, float z);
void ghost_draw(float camZ, uint32_t frame);
int  ghost_count(void);
