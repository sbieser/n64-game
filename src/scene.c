#include <libdragon.h>
#include "scene.h"

/* Forward declarations — each scene file provides these three functions. */
void scene_select_init(void); void scene_select_update(void); void scene_select_draw(void);
void scene_rails_init(void);  void scene_rails_update(void);  void scene_rails_draw(void);
void scene_input_init(void);  void scene_input_update(void);  void scene_input_draw(void);
void scene_ghost_init(void);  void scene_ghost_update(void);  void scene_ghost_draw(void);
void scene_synth_init(void);     void scene_synth_update(void);     void scene_synth_draw(void);
void scene_colossus_init(void);  void scene_colossus_update(void);  void scene_colossus_draw(void);
void scene_seed_init(void);      void scene_seed_update(void);      void scene_seed_draw(void);
void scene_phenomena_init(void); void scene_phenomena_update(void); void scene_phenomena_draw(void);

const Scene scenes[] = {
    { "Select", 0x111122FF, scene_select_init, scene_select_update, scene_select_draw },
    { "Rails",  0x2244AAFF, scene_rails_init,  scene_rails_update,  scene_rails_draw  },
    { "Input",  0x226622FF, scene_input_init,  scene_input_update,  scene_input_draw  },
    { "Ghost",  0x662244FF, scene_ghost_init,  scene_ghost_update,  scene_ghost_draw  },
    { "Synth",    0x226644FF, scene_synth_init,    scene_synth_update,    scene_synth_draw    },
    { "Colossus", 0x221133FF, scene_colossus_init, scene_colossus_update, scene_colossus_draw },
    { "Seed Lab", 0x113322FF, scene_seed_init,     scene_seed_update,     scene_seed_draw     },
    { "Phenom",   0x332211FF, scene_phenomena_init, scene_phenomena_update, scene_phenomena_draw },
};

static const int num_scenes = sizeof(scenes) / sizeof(scenes[0]);
static int current = SCENE_SELECT;

void scene_init(void) {
    rdpq_font_t *font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, font);
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
