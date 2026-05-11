/**
 * N64 Learn — Rails with Obstacles
 *
 * Camera flies forward on a fixed rail through 300 units of space. Five
 * distinct obstacle shapes are scattered along the path. Steer with the
 * analog stick to avoid them. Hitting one flashes the background red.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <math.h>

/* -------------------------------------------------------------------------
 * CONSTANTS
 * -------------------------------------------------------------------------*/
#define RAIL_START      -250.0f
#define RAIL_END          50.0f
#define LATERAL_MAX       20.0f
#define VERTICAL_MAX       6.0f
#define HIT_FLASH_FRAMES  30
#define NUM_OBSTACLES     12

#define SHAPE_CUBE     0
#define SHAPE_TETRA    1
#define SHAPE_OCTA     2
#define SHAPE_PYRAMID  3
#define SHAPE_PRISM    4

/* -------------------------------------------------------------------------
 * OBSTACLE TABLE
 *
 * worldZ   — fixed position along the rail (camera travels from -250 to +50)
 * x, y     — lateral and vertical offset from path center
 * hitRadius — collision sphere radius (play area is ±20 lateral, ±6 vertical)
 * shape     — which geometry to draw
 * rotSpeedX/Y — per-obstacle rotation multipliers on the global angle counter
 * rotPhase  — starting offset so obstacles don't all spin in sync
 * -------------------------------------------------------------------------*/
typedef struct {
    float worldZ, x, y, hitRadius;
    int   shape;
    float rotSpeedX, rotSpeedY, rotPhase;
} Obstacle;

static const Obstacle obstacles[NUM_OBSTACLES] = {
    {-220,  0,  0, 9.0f, SHAPE_CUBE,    0.3f, 1.0f, 0.0f},
    {-195,  8, -2, 9.0f, SHAPE_TETRA,   0.4f, 0.7f, 1.0f},
    {-170, -6,  3, 9.0f, SHAPE_OCTA,    0.2f, 1.2f, 2.1f},
    {-148,  0, -2, 9.0f, SHAPE_PYRAMID, 0.5f, 0.4f, 0.5f},
    {-125, 10,  0, 9.0f, SHAPE_PRISM,   0.3f, 0.8f, 3.2f},
    { -98, -8,  2, 9.0f, SHAPE_CUBE,    0.6f, 0.5f, 1.7f},
    { -74,  0,  4, 9.0f, SHAPE_TETRA,   0.4f, 1.1f, 2.8f},
    { -50,-10, -3, 9.0f, SHAPE_OCTA,    0.2f, 0.9f, 0.3f},
    { -28,  7,  0, 9.0f, SHAPE_PYRAMID, 0.5f, 0.6f, 1.4f},
    {  -8,  0,  0, 9.0f, SHAPE_PRISM,   0.3f, 1.3f, 4.1f},
    {  12, -8,  3, 9.0f, SHAPE_CUBE,    0.4f, 0.7f, 2.2f},
    {  30,  5, -4, 9.0f, SHAPE_OCTA,    0.6f, 0.4f, 3.7f},
};

/* -------------------------------------------------------------------------
 * GEOMETRY
 *
 * Each shape lives in its own uncached vertex buffer. T3DVertPacked holds
 * TWO vertices interleaved — the RSP DMA's them in 32-byte cache-line pairs.
 *
 * All faces are wound CCW when viewed from outside (back-face culling on).
 * Normals use the outward direction of each vertex — approximate for shared
 * vertices, but correct enough for smooth lighting at N64 resolution.
 *
 * Vertex counts and packed-struct counts:
 *   Cube     8 verts → 4 packed structs
 *   Tetra    4 verts → 2 packed structs
 *   Octa     6 verts → 3 packed structs
 *   Pyramid  5 verts → 3 packed structs (last struct uses only slot A)
 *   Prism    6 verts → 3 packed structs
 * -------------------------------------------------------------------------*/

static T3DVertPacked *cubeVerts;
static T3DVertPacked *tetraVerts;
static T3DVertPacked *octaVerts;
static T3DVertPacked *pyramidVerts;
static T3DVertPacked *prismVerts;

static uint16_t packNorm(float x, float y, float z) {
    T3DVec3 v = {{x, y, z}};
    t3d_vec3_norm(&v);
    return t3d_vert_pack_normal(&v);
}

/* -- CUBE ------------------------------------------------------------------
 *
 *   7---6
 *  /|  /|    vertices 0–3: bottom ring (y=-10)
 * 4---5 |    vertices 4–7: top ring    (y=+10)
 * | 3-|-2
 * |/  |/
 * 0---1
 */
static void initCubeVerts(void) {
    const int H = 10;
    cubeVerts[0] = (T3DVertPacked){
        .posA={-H,-H,-H}, .normA=packNorm(-1,-1,-1), .rgbaA=0xFF3333FF,
        .posB={ H,-H,-H}, .normB=packNorm( 1,-1,-1), .rgbaB=0xFFFF33FF,
    };
    cubeVerts[1] = (T3DVertPacked){
        .posA={ H,-H, H}, .normA=packNorm( 1,-1, 1), .rgbaA=0x33FF33FF,
        .posB={-H,-H, H}, .normB=packNorm(-1,-1, 1), .rgbaB=0x33FFFFFF,
    };
    cubeVerts[2] = (T3DVertPacked){
        .posA={-H, H,-H}, .normA=packNorm(-1, 1,-1), .rgbaA=0x3333FFFF,
        .posB={ H, H,-H}, .normB=packNorm( 1, 1,-1), .rgbaB=0xFFFFFFFF,
    };
    cubeVerts[3] = (T3DVertPacked){
        .posA={ H, H, H}, .normA=packNorm( 1, 1, 1), .rgbaA=0xFF8833FF,
        .posB={-H, H, H}, .normB=packNorm(-1, 1, 1), .rgbaB=0xFF33FFFF,
    };
}

static void drawCube(void) {
    t3d_vert_load(cubeVerts, 0, 8);
    t3d_tri_draw(0,5,1); t3d_tri_draw(0,4,5); /* front  z=-10 */
    t3d_tri_draw(2,6,7); t3d_tri_draw(2,7,3); /* back   z=+10 */
    t3d_tri_draw(0,3,7); t3d_tri_draw(0,7,4); /* left   x=-10 */
    t3d_tri_draw(1,5,6); t3d_tri_draw(1,6,2); /* right  x=+10 */
    t3d_tri_draw(0,1,2); t3d_tri_draw(0,2,3); /* bottom y=-10 */
    t3d_tri_draw(4,7,6); t3d_tri_draw(4,6,5); /* top    y=+10 */
    t3d_tri_sync();
}

/* -- TETRAHEDRON -----------------------------------------------------------
 *
 * Apex at top (y=+12). Three base vertices at y=-5:
 *   0=apex, 1=back(+Z), 2=front-left(-X,-Z), 3=front-right(+X,-Z)
 */
static void initTetraVerts(void) {
    tetraVerts[0] = (T3DVertPacked){
        .posA={ 0, 12,  0}, .normA=packNorm( 0, 1, 0), .rgbaA=0x00FFFFFF,
        .posB={ 0, -5, 11}, .normB=packNorm( 0,-1, 1), .rgbaB=0x3333FFFF,
    };
    tetraVerts[1] = (T3DVertPacked){
        .posA={-9, -5, -5}, .normA=packNorm(-1,-1,-1), .rgbaA=0x00AAFFFF,
        .posB={ 9, -5, -5}, .normB=packNorm( 1,-1,-1), .rgbaB=0x8888FFFF,
    };
}

static void drawTetra(void) {
    t3d_vert_load(tetraVerts, 0, 4);
    t3d_tri_draw(1, 2, 3); /* bottom */
    t3d_tri_draw(0, 3, 2); /* front  */
    t3d_tri_draw(0, 1, 3); /* right  */
    t3d_tri_draw(0, 2, 1); /* left   */
    t3d_tri_sync();
}

/* -- OCTAHEDRON ------------------------------------------------------------
 *
 * Two square pyramids base-to-base.
 *   0=top, 1=right(+X), 2=back(+Z), 3=left(-X), 4=front(-Z), 5=bottom
 *
 * Equatorial ring goes CCW from above: 1 → 4 → 3 → 2 → 1
 */
static void initOctaVerts(void) {
    octaVerts[0] = (T3DVertPacked){
        .posA={ 0, 12,  0}, .normA=packNorm( 0, 1, 0), .rgbaA=0x00FF44FF,
        .posB={10,  0,  0}, .normB=packNorm( 1, 0, 0), .rgbaB=0x88FF00FF,
    };
    octaVerts[1] = (T3DVertPacked){
        .posA={ 0,  0, 10}, .normA=packNorm( 0, 0, 1), .rgbaA=0x00CC44FF,
        .posB={-10, 0,  0}, .normB=packNorm(-1, 0, 0), .rgbaB=0x44FFAAFF,
    };
    octaVerts[2] = (T3DVertPacked){
        .posA={ 0,  0,-10}, .normA=packNorm( 0, 0,-1), .rgbaA=0xAAFF00FF,
        .posB={ 0,-12,  0}, .normB=packNorm( 0,-1, 0), .rgbaB=0x007733FF,
    };
}

static void drawOcta(void) {
    t3d_vert_load(octaVerts, 0, 6);
    t3d_tri_draw(0,1,4); t3d_tri_draw(0,4,3); /* top: right-front, front-left */
    t3d_tri_draw(0,3,2); t3d_tri_draw(0,2,1); /* top: left-back,   back-right */
    t3d_tri_draw(5,4,1); t3d_tri_draw(5,3,4); /* bot: right-front, front-left */
    t3d_tri_draw(5,2,3); t3d_tri_draw(5,1,2); /* bot: left-back,   back-right */
    t3d_tri_sync();
}

/* -- SQUARE PYRAMID --------------------------------------------------------
 *
 * Apex at y=+12, square base at y=-5.
 *   0=apex, 1=front-left(-X,-Z), 2=front-right(+X,-Z),
 *   3=back-right(+X,+Z), 4=back-left(-X,+Z)
 */
static void initPyramidVerts(void) {
    pyramidVerts[0] = (T3DVertPacked){
        .posA={ 0, 12,  0}, .normA=packNorm( 0, 1, 0), .rgbaA=0xFFCC00FF,
        .posB={-10,-5,-10}, .normB=packNorm(-1,-1,-1), .rgbaB=0xFF8800FF,
    };
    pyramidVerts[1] = (T3DVertPacked){
        .posA={ 10,-5,-10}, .normA=packNorm( 1,-1,-1), .rgbaA=0xFFAA00FF,
        .posB={ 10,-5, 10}, .normB=packNorm( 1,-1, 1), .rgbaB=0xFFDD44FF,
    };
    pyramidVerts[2] = (T3DVertPacked){
        .posA={-10,-5, 10}, .normA=packNorm(-1,-1, 1), .rgbaA=0xFFBB22FF,
        /* slot B unused — only 5 vertices needed */
    };
}

static void drawPyramid(void) {
    t3d_vert_load(pyramidVerts, 0, 5);
    t3d_tri_draw(1,2,3); t3d_tri_draw(1,3,4); /* base  */
    t3d_tri_draw(0,2,1);                        /* front */
    t3d_tri_draw(0,3,2);                        /* right */
    t3d_tri_draw(0,4,3);                        /* back  */
    t3d_tri_draw(0,1,4);                        /* left  */
    t3d_tri_sync();
}

/* -- TRIANGULAR PRISM ------------------------------------------------------
 *
 * Ridge runs along the X axis. Left triangle at x=-10, right at x=+10.
 *   0=L-front-bot, 1=L-back-bot, 2=L-apex
 *   3=R-front-bot, 4=R-back-bot, 5=R-apex
 */
static void initPrismVerts(void) {
    prismVerts[0] = (T3DVertPacked){
        .posA={-10, 0,-10}, .normA=packNorm(-1,-1,-1), .rgbaA=0xAA33FFFF,
        .posB={-10, 0, 10}, .normB=packNorm(-1,-1, 1), .rgbaB=0x6633CCFF,
    };
    prismVerts[1] = (T3DVertPacked){
        .posA={-10,12,  0}, .normA=packNorm(-1, 1, 0), .rgbaA=0xFF88FFFF,
        .posB={ 10, 0,-10}, .normB=packNorm( 1,-1,-1), .rgbaB=0xCC44FFFF,
    };
    prismVerts[2] = (T3DVertPacked){
        .posA={ 10, 0, 10}, .normA=packNorm( 1,-1, 1), .rgbaA=0x9922CCFF,
        .posB={ 10,12,  0}, .normB=packNorm( 1, 1, 0), .rgbaB=0xEEAAFFFF,
    };
}

static void drawPrism(void) {
    t3d_vert_load(prismVerts, 0, 6);
    t3d_tri_draw(0,1,2);                        /* left face  */
    t3d_tri_draw(3,5,4);                        /* right face */
    t3d_tri_draw(0,3,1); t3d_tri_draw(1,3,4);  /* bottom     */
    t3d_tri_draw(0,2,5); t3d_tri_draw(0,5,3);  /* front slope*/
    t3d_tri_draw(1,5,2); t3d_tri_draw(1,4,5);  /* back slope */
    t3d_tri_sync();
}

static void drawShape(int shape) {
    switch (shape) {
        case SHAPE_CUBE:    drawCube();    break;
        case SHAPE_TETRA:   drawTetra();   break;
        case SHAPE_OCTA:    drawOcta();    break;
        case SHAPE_PYRAMID: drawPyramid(); break;
        case SHAPE_PRISM:   drawPrism();   break;
    }
}

/* -------------------------------------------------------------------------
 * MAIN
 * -------------------------------------------------------------------------*/

int main(void) {
    debug_init_isviewer();
    debug_init_usblog();

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
    t3d_init((T3DInitParams){});
    joypad_init();

    /* Allocate all vertex buffers in uncached RDRAM. The RSP DMA engine reads
     * directly from RDRAM — if these were in cached memory the RSP might see
     * stale data that hasn't been flushed from the CPU cache yet. */
    cubeVerts    = malloc_uncached(sizeof(T3DVertPacked) * 4);
    tetraVerts   = malloc_uncached(sizeof(T3DVertPacked) * 2);
    octaVerts    = malloc_uncached(sizeof(T3DVertPacked) * 3);
    pyramidVerts = malloc_uncached(sizeof(T3DVertPacked) * 3);
    prismVerts   = malloc_uncached(sizeof(T3DVertPacked) * 3);

    initCubeVerts();
    initTetraVerts();
    initOctaVerts();
    initPyramidVerts();
    initPrismVerts();

    /* One model matrix per obstacle in a single contiguous uncached block.
     * rdpq_detach_show() at the end of each frame acts as a full pipeline
     * sync — the RSP is guaranteed done with all matrices before the next
     * frame's CPU writes begin, so one matrix per obstacle (not three) is safe. */
    T3DMat4FP *obsMats = malloc_uncached(sizeof(T3DMat4FP) * NUM_OBSTACLES);

    T3DViewport viewport = t3d_viewport_create();

    uint8_t ambientColor[4]     = {40,  40,  80, 0xFF};
    uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
    T3DVec3 lightDir = {{1.0f, 1.0f, -0.5f}};
    t3d_vec3_norm(&lightDir);

    float railZ       = RAIL_START;
    float rotAngle    = 0.0f;
    float lateralPos  = 0.0f;
    float verticalPos = 0.0f;
    int   hitFlash    = 0;

    for (;;) {
        /* ---- UPDATE ---- */

        joypad_poll();
        joypad_inputs_t pad = joypad_get_inputs(JOYPAD_PORT_1);

        /* Velocity-based steering: holding the stick accelerates the camera
         * sideways each frame rather than snapping to a fixed offset. */
        lateralPos  += (pad.stick_x / 85.0f) * 0.5f;
        verticalPos += (pad.stick_y / 85.0f) * 0.5f;
        if (lateralPos  >  LATERAL_MAX)  lateralPos  =  LATERAL_MAX;
        if (lateralPos  < -LATERAL_MAX)  lateralPos  = -LATERAL_MAX;
        if (verticalPos >  VERTICAL_MAX) verticalPos =  VERTICAL_MAX;
        if (verticalPos < -VERTICAL_MAX) verticalPos = -VERTICAL_MAX;

        railZ    += 0.4f;
        rotAngle += 0.02f;
        if (railZ > RAIL_END) railZ = RAIL_START;

        /* Build each obstacle's model matrix from its world position plus
         * its individual rotation speed and phase. */
        for (int i = 0; i < NUM_OBSTACLES; i++) {
            const Obstacle *o = &obstacles[i];
            float angle = rotAngle + o->rotPhase;
            t3d_mat4fp_from_srt_euler(&obsMats[i],
                (float[3]){1.0f, 1.0f, 1.0f},
                (float[3]){o->rotSpeedX * angle, o->rotSpeedY * angle, 0.0f},
                (float[3]){o->x, o->y, o->worldZ}
            );
        }

        /* Collision — camera sits 15 units behind railZ on the Z axis.
         * We check a flat cylinder: within ±8 units in Z and within hitRadius
         * in the XY (lateral/vertical) plane. Squaring both sides avoids
         * a sqrtf every frame. */
        float camZ = railZ - 15.0f;
        if (hitFlash > 0) {
            hitFlash--;
        } else {
            for (int i = 0; i < NUM_OBSTACLES; i++) {
                const Obstacle *o = &obstacles[i];
                float dz = camZ - o->worldZ;
                if (dz > -12.0f && dz < 2.0f) {
                    float dx = lateralPos - o->x;
                    float dy = verticalPos - o->y;
                    if (dx*dx + dy*dy < o->hitRadius * o->hitRadius) {
                        hitFlash = HIT_FLASH_FRAMES;
                    }
                }
            }
        }

        /* Camera follows the stick. The look-at target uses smaller lateral
         * multipliers so the camera tilts into turns rather than sliding flat. */
        T3DVec3 camPos    = {{lateralPos,        8.0f + verticalPos, camZ}};
        T3DVec3 camTarget = {{lateralPos * 0.6f, verticalPos * 0.4f, railZ + 10.0f}};

        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 10.0f, 200.0f);
        t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

        /* ---- DRAW ---- */

        rdpq_attach(display_get(), display_get_zbuf());
        t3d_frame_start();
        t3d_viewport_attach(&viewport);
        rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

        /* Flash red on hit — just change the sky color, simple and effective. */
        t3d_screen_clear_color(hitFlash > 0
            ? RGBA32(100, 20, 20, 0xFF)
            : RGBA32( 50, 50,100, 0xFF));
        t3d_screen_clear_depth();

        /* Directional lights must be re-applied after each t3d_viewport_attach
         * because the view matrix affects how the RSP transforms them. */
        t3d_light_set_ambient(ambientColor);
        t3d_light_set_directional(0, directionalColor, &lightDir);
        t3d_light_set_count(1);

        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);

        /* Draw all obstacles. Push matrix → load and transform vertices → pop.
         * The pop can come after the tri draws because vertices are already
         * in the RSP cache, transformed; the matrix stack is no longer needed. */
        for (int i = 0; i < NUM_OBSTACLES; i++) {
            t3d_matrix_push(&obsMats[i]);
            drawShape(obstacles[i].shape);
            t3d_matrix_pop(1);
        }

        /* Flip to screen. This syncs the full RSP/RDP pipeline — safe to
         * overwrite obsMats[] on the next frame. */
        rdpq_detach_show();
    }

    t3d_destroy();
    return 0;
}
