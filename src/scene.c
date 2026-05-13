#include "scene.h"

/* Forward declarations — each scene file provides these three functions. */
void scene_select_init(void); void scene_select_update(void); void scene_select_draw(void);
void scene_rails_init(void);  void scene_rails_update(void);  void scene_rails_draw(void);
void scene_input_init(void);  void scene_input_update(void);  void scene_input_draw(void);

const Scene scenes[] = {
    { "Select", 0x111122FF, scene_select_init, scene_select_update, scene_select_draw },
    { "Rails",  0x2244AAFF, scene_rails_init,  scene_rails_update,  scene_rails_draw  },
    { "Input",  0x226622FF, scene_input_init,  scene_input_update,  scene_input_draw  },
};

static const int num_scenes = sizeof(scenes) / sizeof(scenes[0]);
static int current = SCENE_SELECT;

void scene_init(void) {
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
