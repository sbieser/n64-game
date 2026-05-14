/**
 * shapes.c — Manual geometry for the five obstacle shapes.
 *
 * Each shape owns its own uncached vertex buffer. shapes_init() allocates
 * and fills them all. draw_shape() dispatches to the right draw function.
 *
 * T3DVertPacked holds TWO vertices interleaved so the RSP DMA can fill a
 * full 32-byte cache line in one burst. All faces are wound CCW from outside
 * to work with T3D_FLAG_CULL_BACK. Normals use the outward corner direction
 * of each vertex (approximate for shared vertices, fine at N64 resolution).
 */

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "shapes.h"

static T3DVertPacked *cubeVerts;     /* 4 packed = 8 verts */
static T3DVertPacked *tetraVerts;    /* 2 packed = 4 verts */
static T3DVertPacked *octaVerts;     /* 3 packed = 6 verts */
static T3DVertPacked *pyramidVerts;  /* 3 packed = 5 verts (last slot B unused) */
static T3DVertPacked *prismVerts;    /* 3 packed = 6 verts */

static uint16_t packNorm(float x, float y, float z) {
    T3DVec3 v = {{x, y, z}};
    t3d_vec3_norm(&v);
    return t3d_vert_pack_normal(&v);
}

/* -- CUBE ------------------------------------------------------------------
 *   7---6
 *  /|  /|    0–3: bottom ring (y=-10)
 * 4---5 |    4–7: top ring    (y=+10)
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
 * Apex at top (y=+12). Base at y=-5:
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
 * Two square pyramids base-to-base.
 *   0=top, 1=right(+X), 2=back(+Z), 3=left(-X), 4=front(-Z), 5=bottom
 * Equatorial ring CCW from above: 1 → 4 → 3 → 2 → 1
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
 * Ridge along the X axis. Left triangle at x=-10, right at x=+10.
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
    t3d_tri_draw(0,1,2);                        /* left face   */
    t3d_tri_draw(3,5,4);                        /* right face  */
    t3d_tri_draw(0,3,1); t3d_tri_draw(1,3,4);  /* bottom      */
    t3d_tri_draw(0,2,5); t3d_tri_draw(0,5,3);  /* front slope */
    t3d_tri_draw(1,5,2); t3d_tri_draw(1,4,5);  /* back slope  */
    t3d_tri_sync();
}

/* ---- PUBLIC API ---- */

void shapes_init(void) {
    static bool done = false;
    if (done) return;
    done = true;
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
}

void draw_shape(int shape) {
    switch (shape) {
        case SHAPE_CUBE:    drawCube();    break;
        case SHAPE_TETRA:   drawTetra();   break;
        case SHAPE_OCTA:    drawOcta();    break;
        case SHAPE_PYRAMID: drawPyramid(); break;
        case SHAPE_PRISM:   drawPrism();   break;
    }
}
