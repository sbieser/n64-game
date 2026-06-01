#pragma once
#include <stdint.h>

/*
 * scene.h — the scene framework.
 *
 * Every screen in the build is a Scene: a name, a selector tile color, and
 * three callbacks (init / update / draw).  scene.c holds the registry of all
 * of them and runs exactly one at a time.  See scene.c for how dispatch works.
 *
 * The SCENE_* constants are indices into that registry — pass them to
 * scene_switch().  The registry is built with these as designated initializers
 * so the index and the constant can never disagree.
 */

typedef struct {
    const char *name;        /* display name, shown on the selector tile */
    uint32_t    color;       /* packed RGBA32 — tile color in selector */
    void (*init)(void);      /* called once on entry (scene_switch) */
    void (*update)(void);    /* called every frame — game logic + input */
    void (*draw)(void);      /* called every frame — rendering */
} Scene;

#define SCENE_SELECT  0
#define SCENE_RAILS   1
#define SCENE_INPUT   2
#define SCENE_GHOST   3
#define SCENE_SYNTH      4
#define SCENE_COLOSSUS   5
#define SCENE_SEED       6
#define SCENE_PHENOMENA  7
#define SCENE_STARGATE   8
#define SCENE_ICE_MOON   9
#define SCENE_STAR_FIELD 10
#define SCENE_VOID       11

/* Initialize the framework (fonts, cockpit) and enter SCENE_SELECT. Call once. */
void         scene_init(void);

/* Run the current scene's update() then draw(). Call once per frame. */
void         scene_tick(void);

/* Switch to scene n (a SCENE_* constant) and call its init(). Out-of-range is ignored. */
void         scene_switch(int n);

/* Total number of registered scenes. */
int          scene_count(void);

/* Borrow a pointer to scene n's registry entry (name/color/callbacks). */
const Scene *scene_get(int n);
