#pragma once
#include <stdint.h>

typedef struct {
    const char *name;
    uint32_t    color;   /* packed RGBA32 — tile color in selector */
    void (*init)(void);
    void (*update)(void);
    void (*draw)(void);
} Scene;

#define SCENE_SELECT  0
#define SCENE_RAILS   1
#define SCENE_INPUT   2
#define SCENE_GHOST   3
#define SCENE_SYNTH      4
#define SCENE_COLOSSUS   5

void         scene_init(void);
void         scene_tick(void);
void         scene_switch(int n);
int          scene_count(void);
const Scene *scene_get(int n);
