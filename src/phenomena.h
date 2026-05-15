#pragma once
#include <stdint.h>

void  phenomena_generate(uint32_t seed);
void  phenomena_update(float railZ, float *lateralPos);
float phenomena_pull(void);

int   phenomena_field_count(void);
void  phenomena_get_field(int i, float *z_start, float *z_end, float *pull);
