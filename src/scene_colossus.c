/*
 * scene_colossus.c — The Colossus: a breathing geometric entity
 *
 * This scene implements the centrepiece of the game design: a massive
 * geometric form with per-vertex displacement that makes it feel alive.
 * Every frame, each of the 12 vertices is pushed outward along its normal
 * by a sum of sine waves — different frequencies, different speeds — so
 * the surface breathes in complex, never-repeating patterns.
 *
 * THE BASE GEOMETRY — ICOSAHEDRON
 * ────────────────────────────────
 * An icosahedron has 12 vertices and 20 equilateral triangular faces.
 * It is the platonic solid closest to a sphere with the fewest faces —
 * good for N64 where every triangle has a cost. Scaled to radius ~60
 * world units, each face is roughly 63 units across.
 *
 * The 12 vertices sit on a sphere using the golden ratio (φ ≈ 1.618):
 *   (0, ±1, ±φ), (±1, ±φ, 0), (±φ, 0, ±1)  — normalized then scaled
 * This produces a perfectly symmetric form with 5-fold symmetry
 * around the Y axis, which gives the displacement an organic rhythm.
 *
 * VERTEX DISPLACEMENT — THE BREATHING
 * ─────────────────────────────────────
 * Each frame, every vertex is displaced outward along its normal by:
 *
 *   d = (0.3*sin(x*0.8 + t*0.40) +
 *        0.2*sin(y*1.1 + t*0.27) +
 *        0.1*sin(z*1.4 + t*0.13)) * 8.0
 *
 *   new_pos = base_pos + normal * d
 *
 * Three sine waves with different spatial frequencies (0.8, 1.1, 1.4)
 * and different temporal rates (0.40, 0.27, 0.13). Because the rates
 * are irrational multiples of each other, the combination never exactly
 * repeats — it evolves continuously like a living surface.
 *
 * The spatial terms (x*0.8 etc.) mean vertices at different positions
 * on the mesh get different phase values, so neighbouring vertices pulse
 * at different offsets. This creates the rippling, breathing appearance.
 * The scale factor of 8.0 gives ±6 world units of displacement on a
 * radius-60 sphere — about 10% deformation, visible but not violent.
 *
 * WHY DISPLACEMENT IN UPDATE, NOT DRAW?
 * ──────────────────────────────────────
 * The RSP DMA's vertex data from RDRAM into its internal cache.
 * Once t3d_vert_load() is called, the RSP processes those vertices
 * asynchronously. We must have the displaced positions ready in RDRAM
 * before issuing the load — so the computation happens in update()
 * and the draw() call just issues the RSP commands.
 *
 * WHY NO BACK-FACE CULLING?
 * ─────────────────────────
 * T3D_FLAG_CULL_BACK discards triangles whose winding appears clockwise
 * from the camera (back-facing). It's an optimization — roughly half the
 * faces of a closed convex mesh face away at any given time.
 * We omit it here because ensuring consistent CCW winding across all
 * 20 manually-defined faces would require careful checking. At 20 triangles
 * total, the RDP cost of drawing the extra back-faces is negligible.
 * If performance mattered, fixing the winding order would halve draw cost.
 *
 * ORBIT CAMERA
 * ────────────
 * Stick X rotates the camera around the Y axis (azimuth angle).
 * Stick Y moves it closer or further (radial distance).
 * Position computed as:
 *   camX = sin(angle) * dist
 *   camZ = -cos(angle) * dist
 * The camera always looks toward (0, 10, 0) — slightly above the centre
 * of the Colossus, so the view is looking up at a towering form.
 *
 * LIGHTING
 * ────────
 * Almost-black ambient (8, 8, 20) — deep space with the faintest blue.
 * Cool directional (160, 160, 220) from upper-left — starlight.
 * The result: most of the Colossus is near-black, with lit faces
 * showing cold blue-white highlights. Ancient and vast.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <math.h>
#include "scene.h"

/* Icosahedron geometry — radius ≈ 60 world units
   S = 60 * 0.5257 (sin of icosahedron latitude angle)
   L = 60 * 0.8507 (cos of icosahedron latitude angle) */
#define S 31.54f
#define L 51.04f

static const float BASE_POS[12][3] = {
    { 0, S, L}, { 0,-S, L}, { 0, S,-L}, { 0,-S,-L},
    { S, L, 0}, {-S, L, 0}, { S,-L, 0}, {-S,-L, 0},
    { L, 0, S}, {-L, 0, S}, { L, 0,-S}, {-L, 0,-S},
};

/* Unit normals — same direction as positions since they sit on a sphere */
static const float BASE_NORM[12][3] = {
    { 0,      0.5257f, 0.8507f}, { 0,     -0.5257f, 0.8507f},
    { 0,      0.5257f,-0.8507f}, { 0,     -0.5257f,-0.8507f},
    { 0.5257f, 0.8507f, 0},     {-0.5257f, 0.8507f, 0},
    { 0.5257f,-0.8507f, 0},     {-0.5257f,-0.8507f, 0},
    { 0.8507f, 0,      0.5257f},{-0.8507f, 0,       0.5257f},
    { 0.8507f, 0,     -0.5257f},{-0.8507f, 0,      -0.5257f},
};

/* 20 triangular faces as vertex index triplets */
static const uint8_t FACES[20][3] = {
    /* top cap (5 faces around vertex 0) */
    {0,4,8},{0,8,1},{0,1,9},{0,9,5},{0,5,4},
    /* upper equatorial band */
    {4,5,2},{4,2,10},{4,10,8},{8,10,6},{8,6,1},
    /* lower equatorial band */
    {1,6,7},{1,7,9},{9,7,11},{9,11,5},{5,11,2},
    /* bottom cap (5 faces around vertex 3) */
    {3,2,11},{3,11,7},{3,7,6},{3,6,10},{3,10,2},
};

static T3DViewport    viewport;
static T3DMat4FP     *modelMat;
static T3DVertPacked *workVerts;  /* 6 packed = 12 vertices, rebuilt each frame */
static bool           initialized = false;

static float timeVal  = 0.0f;
static float camAngle = 0.0f;
static float camDist  = 110.0f;

static uint8_t ambientColor[4]     = {  8,   8,  20, 0xFF};
static uint8_t directionalColor[4] = {160, 160, 220, 0xFF};
static T3DVec3 lightDir;

static uint16_t packNorm(float x, float y, float z) {
    T3DVec3 v = {{x, y, z}};
    t3d_vec3_norm(&v);
    return t3d_vert_pack_normal(&v);
}

void scene_colossus_init(void) {
    if (!initialized) {
        workVerts = malloc_uncached(sizeof(T3DVertPacked) * 6);
        modelMat  = malloc_uncached(sizeof(T3DMat4FP));
        viewport  = t3d_viewport_create();
        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 5.0f, 250.0f);
        lightDir = (T3DVec3){{-0.5f, 1.0f, -0.8f}};
        t3d_vec3_norm(&lightDir);
        initialized = true;
    }
    timeVal  = 0.0f;
    camAngle = 0.0f;
    camDist  = 110.0f;
}

void scene_colossus_update(void) {
    joypad_poll();
    joypad_inputs_t  pad = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    if (btn.b) { scene_switch(SCENE_SELECT); return; }

    camAngle += (pad.stick_x / 85.0f) * 0.03f;
    camDist  -= (pad.stick_y / 85.0f) * 0.8f;
    if (camDist <  25.0f) camDist =  25.0f;
    if (camDist > 200.0f) camDist = 200.0f;

    timeVal += 0.016f;

    /* Orbit camera position */
    float camX = sinf(camAngle) * camDist;
    float camZ = -cosf(camAngle) * camDist;
    T3DVec3 camPos    = {{camX, -15.0f, camZ}};
    T3DVec3 camTarget = {{ 0.0f,  10.0f, 0.0f}};
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    t3d_mat4fp_from_srt_euler(modelMat,
        (float[3]){1.0f, 1.0f, 1.0f},
        (float[3]){0.0f, 0.0f, 0.0f},
        (float[3]){0.0f, 0.0f, 0.0f}
    );

    /* Compute displaced vertex positions for this frame */
    float t = timeVal;
    for (int i = 0; i < 12; i++) {
        float bx = BASE_POS[i][0], by = BASE_POS[i][1], bz = BASE_POS[i][2];
        float nx = BASE_NORM[i][0], ny = BASE_NORM[i][1], nz = BASE_NORM[i][2];

        float d = (0.3f * sinf(bx * 0.8f + t * 0.40f)
                 + 0.2f * sinf(by * 1.1f + t * 0.27f)
                 + 0.1f * sinf(bz * 1.4f + t * 0.13f)) * 8.0f;

        int16_t px = (int16_t)(bx + nx * d);
        int16_t py = (int16_t)(by + ny * d);
        int16_t pz = (int16_t)(bz + nz * d);
        uint16_t norm = packNorm(nx, ny, nz);

        int pair = i >> 1;
        if ((i & 1) == 0) {
            workVerts[pair].posA[0] = px; workVerts[pair].posA[1] = py; workVerts[pair].posA[2] = pz;
            workVerts[pair].normA   = norm;
            workVerts[pair].rgbaA   = 0x6644AAFF;
        } else {
            workVerts[pair].posB[0] = px; workVerts[pair].posB[1] = py; workVerts[pair].posB[2] = pz;
            workVerts[pair].normB   = norm;
            workVerts[pair].rgbaB   = 0x6644AAFF;
        }
    }
}

void scene_colossus_draw(void) {
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start();
    t3d_viewport_attach(&viewport);
    rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

    t3d_screen_clear_color(RGBA32(2, 2, 8, 0xFF));
    t3d_screen_clear_depth();

    /* No back-face culling — see comment at top of file */
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH);
    t3d_light_set_ambient(ambientColor);
    t3d_light_set_directional(0, directionalColor, &lightDir);
    t3d_light_set_count(1);

    t3d_matrix_push(modelMat);
    t3d_vert_load(workVerts, 0, 12);
    for (int f = 0; f < 20; f++) {
        t3d_tri_draw(FACES[f][0], FACES[f][1], FACES[f][2]);
    }
    t3d_tri_sync();
    t3d_matrix_pop(1);

    rdpq_text_print(NULL, 1, 8,  14, "THE COLOSSUS");
    rdpq_text_printf(NULL, 1, 8,  26, "dist:%.0f", camDist);
    rdpq_text_print(NULL, 1, 8, 228, "X orbit  Y zoom  B back");

    rdpq_detach_show();
}
