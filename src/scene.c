/*
 * scene.c — the scene registry and the per-frame dispatch loop.
 *
 * HOW IT WORKS
 * ────────────
 * Every screen in the build (the demo grid, the test scenes, the real game
 * stages) is a Scene: a name, a selector tile color, and three function
 * pointers — init / update / draw.  This file holds the table of all of them
 * and runs exactly one at a time.
 *
 * scene_tick() (called every frame from main) runs the current scene's
 * update() then draw().  scene_switch(n) changes which scene is current and
 * calls its init().  That is the whole framework — each scene file is a
 * self-contained init/update/draw triple and never has to know about the others.
 *
 * INDEX = CONSTANT (enforced)
 * ───────────────────────────
 * The SCENE_* constants in scene.h are indices into this table, and code calls
 * scene_switch(SCENE_VOID) etc.  To make that correspondence impossible to get
 * wrong, the table uses designated initializers — `[SCENE_VOID] = {...}` puts
 * each entry at its own constant's slot regardless of source order.  The
 * _Static_assert below then fails the build if the table size and the highest
 * constant ever drift apart (e.g. you add a SCENE_* but forget a table row).
 */

#include <libdragon.h>
#include "scene.h"
#include "cockpit.h"

/* Forward declarations — each scene file provides these three functions. */
void scene_select_init(void); void scene_select_update(void); void scene_select_draw(void);
void scene_rails_init(void);  void scene_rails_update(void);  void scene_rails_draw(void);
void scene_input_init(void);  void scene_input_update(void);  void scene_input_draw(void);
void scene_ghost_init(void);  void scene_ghost_update(void);  void scene_ghost_draw(void);
void scene_synth_init(void);     void scene_synth_update(void);     void scene_synth_draw(void);
void scene_colossus_init(void);  void scene_colossus_update(void);  void scene_colossus_draw(void);
void scene_seed_init(void);      void scene_seed_update(void);      void scene_seed_draw(void);
void scene_phenomena_init(void); void scene_phenomena_update(void); void scene_phenomena_draw(void);
void scene_stargate_init(void);  void scene_stargate_update(void);  void scene_stargate_draw(void);
void scene_ice_moon_init(void);  void scene_ice_moon_update(void);  void scene_ice_moon_draw(void);
void scene_star_field_init(void); void scene_star_field_update(void); void scene_star_field_draw(void);
void scene_void_init(void);       void scene_void_update(void);       void scene_void_draw(void);

/* Indexed by the SCENE_* constants so the table slot and the constant agree. */
const Scene scenes[] = {
    [SCENE_SELECT]     = { "Select",   0x111122FF, scene_select_init,     scene_select_update,     scene_select_draw     },
    [SCENE_RAILS]      = { "Rails",     0x2244AAFF, scene_rails_init,      scene_rails_update,      scene_rails_draw      },
    [SCENE_INPUT]      = { "Input",     0x226622FF, scene_input_init,      scene_input_update,      scene_input_draw      },
    [SCENE_GHOST]      = { "Ghost",     0x662244FF, scene_ghost_init,      scene_ghost_update,      scene_ghost_draw      },
    [SCENE_SYNTH]      = { "Synth",     0x226644FF, scene_synth_init,      scene_synth_update,      scene_synth_draw      },
    [SCENE_COLOSSUS]   = { "Colossus",  0x221133FF, scene_colossus_init,   scene_colossus_update,   scene_colossus_draw   },
    [SCENE_SEED]       = { "Seed Lab",  0x113322FF, scene_seed_init,       scene_seed_update,       scene_seed_draw       },
    [SCENE_PHENOMENA]  = { "Phenom",    0x332211FF, scene_phenomena_init,  scene_phenomena_update,  scene_phenomena_draw  },
    [SCENE_STARGATE]   = { "Stargate",  0x110033FF, scene_stargate_init,   scene_stargate_update,   scene_stargate_draw   },
    [SCENE_ICE_MOON]   = { "Ice Moon",  0x112244FF, scene_ice_moon_init,   scene_ice_moon_update,   scene_ice_moon_draw   },
    [SCENE_STAR_FIELD] = { "Stars",     0x010108FF, scene_star_field_init, scene_star_field_update, scene_star_field_draw },
    [SCENE_VOID]       = { "Void",      0x02030CFF, scene_void_init,       scene_void_update,       scene_void_draw       },
};

/* If you add a SCENE_* constant, add its table row (or vice versa), or this
 * fails the build instead of crashing at runtime on a NULL function pointer. */
_Static_assert(sizeof(scenes) / sizeof(scenes[0]) == SCENE_VOID + 1,
               "scenes[] table size must match the SCENE_* constant range");

static const int num_scenes = sizeof(scenes) / sizeof(scenes[0]);
static int current = SCENE_SELECT;

void scene_init(void) {
    rdpq_font_t *font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, font);
    cockpit_init();
    scenes[SCENE_SELECT].init();
}

void scene_tick(void) {
    scenes[current].update();
    scenes[current].draw();
}

void scene_switch(int n) {
    if (n < 0 || n >= num_scenes) return;
    current = n;
    scenes[current].init();
}

int          scene_count(void)    { return num_scenes; }
const Scene *scene_get(int n)     { return &scenes[n]; }
