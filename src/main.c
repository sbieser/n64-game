/**
 * N64 Learn — Rails Movement
 *
 * Step 1 toward The Colossus: the camera flies forward along a fixed rail
 * through space. The cube sits still as a fixed obstacle. This establishes
 * the core feel of the game — propulsion toward something, the sense that
 * you are moving and the universe is not.
 */

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

/* -------------------------------------------------------------------------
 * CUBE GEOMETRY
 *
 * The N64's RSP (Reality Signal Processor) works with vertices loaded into a
 * small on-chip cache (70 slots max). We define our 8 corners here, packed
 * into T3DVertPacked structs. Each struct holds TWO vertices interleaved —
 * this layout is required so a single DMA burst fills a full 32-byte cache
 * line on the RSP.
 *
 * Corner layout (Y-up, right-handed):
 *
 *      7 ------- 6
 *     /|         /|
 *    4 ------- 5  |
 *    |  3 -----|- 2
 *    | /       | /
 *    0 ------- 1
 *
 * Normals are the outward diagonal at each corner (e.g. corner 0 at
 * (-1,-1,-1) gets normal (-1,-1,-1) normalized). Lighting will be smooth
 * across faces — proper flat shading would require duplicating vertices
 * so each face has its own set, but 8 shared vertices teaches the concept
 * more clearly.
 *
 * Vertices must live in UNCACHED memory. The RSP's DMA engine reads directly
 * from RDRAM, bypassing the CPU cache. If this were cached memory, the CPU's
 * writes might not have flushed to RDRAM yet when the RSP reads, causing the
 * RSP to see stale data.
 * -------------------------------------------------------------------------*/

#define CUBE_HALF 10  /* half-size of the cube in world units */

static T3DVertPacked *cubeVerts; /* allocated in main() via malloc_uncached */

/* Packs a corner normal from a float direction. The RSP stores normals in
 * 5.6.5 bit format (5 bits X, 6 bits Y, 5 bits Z) to save DMEM space. */
static uint16_t packNorm(float x, float y, float z) {
    T3DVec3 v = {{x, y, z}};
    t3d_vec3_norm(&v);
    return t3d_vert_pack_normal(&v);
}

static void initCubeVerts(void) {
    /* Each T3DVertPacked holds two vertices. Fields:
     *   posA/posB  — s16 position (world units, integer part of s16.16)
     *   normA/normB — packed normal (for lighting)
     *   rgbaA/rgbaB — vertex color 0xRRGGBBAA
     *   stA/stB    — UV coords (unused here, left as 0) */
    cubeVerts[0] = (T3DVertPacked){
        .posA  = {-CUBE_HALF, -CUBE_HALF, -CUBE_HALF}, /* vert 0: front-bottom-left  */
        .normA = packNorm(-1,-1,-1),
        .rgbaA = 0xFF3333FF,                            /* red    */
        .posB  = { CUBE_HALF, -CUBE_HALF, -CUBE_HALF}, /* vert 1: front-bottom-right */
        .normB = packNorm( 1,-1,-1),
        .rgbaB = 0xFFFF33FF,                            /* yellow */
    };
    cubeVerts[1] = (T3DVertPacked){
        .posA  = { CUBE_HALF, -CUBE_HALF,  CUBE_HALF}, /* vert 2: back-bottom-right  */
        .normA = packNorm( 1,-1, 1),
        .rgbaA = 0x33FF33FF,                            /* green  */
        .posB  = {-CUBE_HALF, -CUBE_HALF,  CUBE_HALF}, /* vert 3: back-bottom-left   */
        .normB = packNorm(-1,-1, 1),
        .rgbaB = 0x33FFFFFF,                            /* cyan   */
    };
    cubeVerts[2] = (T3DVertPacked){
        .posA  = {-CUBE_HALF,  CUBE_HALF, -CUBE_HALF}, /* vert 4: front-top-left     */
        .normA = packNorm(-1, 1,-1),
        .rgbaA = 0x3333FFFF,                            /* blue   */
        .posB  = { CUBE_HALF,  CUBE_HALF, -CUBE_HALF}, /* vert 5: front-top-right    */
        .normB = packNorm( 1, 1,-1),
        .rgbaB = 0xFFFFFFFF,                            /* white  */
    };
    cubeVerts[3] = (T3DVertPacked){
        .posA  = { CUBE_HALF,  CUBE_HALF,  CUBE_HALF}, /* vert 6: back-top-right     */
        .normA = packNorm( 1, 1, 1),
        .rgbaA = 0xFF8833FF,                            /* orange  */
        .posB  = {-CUBE_HALF,  CUBE_HALF,  CUBE_HALF}, /* vert 7: back-top-left      */
        .normB = packNorm(-1, 1, 1),
        .rgbaB = 0xFF33FFFF,                            /* magenta */
    };
}

/* -------------------------------------------------------------------------
 * MAIN
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Debug output — readable in Ares via Tools → Debug Log, and over USB on
     * real hardware. Always include this during development. */
    debug_init_isviewer();
    debug_init_usblog();

    /* Initialize the display: 320x240 pixels, 16-bit color (RGB565), 3
     * framebuffers (triple buffering), no gamma correction, resample AA.
     *
     * Triple buffering: the TV is showing buffer A, the RDP is drawing into
     * buffer B, and the CPU is building commands for buffer C. This prevents
     * tearing and keeps the pipeline full. */
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    /* rdpq is libdragon's RDP command queue — the RDP is the rasterizer that
     * turns triangles into pixels. We never talk to the RDP directly; we queue
     * commands and the hardware drains the queue. */
    rdpq_init();

    /* Initialize tiny3d. The empty T3DInitParams uses all defaults (matrix
     * stack depth of 8, etc.). */
    t3d_init((T3DInitParams){});

    /* Allocate and fill cube vertices in uncached memory. */
    cubeVerts = malloc_uncached(sizeof(T3DVertPacked) * 4);
    initCubeVerts();

    /* The model matrix moves/rotates/scales the cube in world space.
     *
     * We keep two representations:
     *   modelMat   — float (T3DMat4), easy to build with standard math
     *   modelMatFP — fixed-point (T3DMat4FP), what the RSP actually reads
     *
     * modelMatFP must also be uncached — the RSP DMA's it each frame. */
    T3DMat4    modelMat;
    T3DMat4FP *modelMatFP = malloc_uncached(sizeof(T3DMat4FP));

    /* A viewport holds the camera (view matrix) and lens (projection matrix).
     * t3d_viewport_create() initializes it to the full screen size. */
    T3DViewport viewport = t3d_viewport_create();

    /* Lighting: a dim blue ambient so nothing is fully black, plus one warm
     * directional light from the upper-right. */
    uint8_t ambientColor[4]     = {40,  40,  80,  0xFF};
    uint8_t directionalColor[4] = {200, 200, 160, 0xFF};
    T3DVec3 lightDir = {{1.0f, 1.0f, -0.5f}};
    t3d_vec3_norm(&lightDir);

    /* Rail position — the camera's Z coordinate along the forward axis.
     * Starts far back, advances each frame. The cube sits at Z=0.
     * When the camera passes the cube we loop back to the start. */
    float railZ = -80.0f;

    /* The cube doesn't move, so its model matrix is a fixed identity.
     * Set it once here — no need to rebuild it every frame. */
    t3d_mat4_identity(&modelMat);
    t3d_mat4_to_fixed(modelMatFP, &modelMat);

    /* Display list: records RSP commands once, replayed every frame.
     *
     * The N64 CPU and RSP run concurrently. Rather than sending one command at
     * a time and waiting, we build a buffer of commands (the display list), then
     * hand the whole buffer to the RSP in one go. The RSP executes it while the
     * CPU is already preparing the next frame. This is the fundamental N64
     * rendering pattern. */
    rspq_block_t *dplDraw = NULL;

    for (;;) {
        /* ---- UPDATE ---- */

        /* Advance the camera along the rail. 0.4 units/frame at 60fps =
         * 24 units/second. The cube is 20 units wide so it takes roughly
         * 5 seconds to fly from first sight to passing through it. */
        railZ += 0.4f;
        if (railZ > 30.0f) railZ = -80.0f; /* loop back for testing */

        /* Camera sits 15 units behind and 8 units above the rail position,
         * looking 10 units ahead. As railZ increases the whole window slides
         * forward — the cube ahead grows, passes, disappears behind.
         *
         * This is the simplest possible rails camera: a fixed offset from a
         * moving point. In the real game the player's lateral input will shift
         * the camera left/right/up/down within this window. */
        T3DVec3 camPos    = {{0.0f, 8.0f, railZ - 15.0f}};
        T3DVec3 camTarget = {{0.0f, 0.0f, railZ + 10.0f}};

        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(70.0f), 10.0f, 200.0f);
        t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

        /* ---- DRAW ---- */

        /* Target the current framebuffer + its depth buffer. display_get()
         * rotates through the 3 buffers automatically. */
        rdpq_attach(display_get(), display_get_zbuf());

        /* Reset RSP state for a new frame. */
        t3d_frame_start();

        /* Apply the viewport: uploads the projection and view matrices to the
         * RSP, and sets the scissor rectangle to the screen bounds. */
        t3d_viewport_attach(&viewport);

        /* Tell the RDP to shade pixels using vertex colors (interpolated across
         * each triangle). RDPQ_COMBINER_SHADE is the simplest mode: final pixel
         * color = vertex color * light. */
        rdpq_mode_combiner(RDPQ_COMBINER_SHADE);

        /* Clear the color buffer (dark navy) and the depth buffer. The depth
         * buffer stores how far each pixel is from the camera so that closer
         * triangles correctly overdraw farther ones. */
        t3d_screen_clear_color(RGBA32(10, 10, 30, 0xFF));
        t3d_screen_clear_depth();

        /* Set up lighting. Directional lights must be re-applied after each
         * t3d_viewport_attach because the view matrix affects how the RSP
         * transforms the light direction into view space. */
        t3d_light_set_ambient(ambientColor);
        t3d_light_set_directional(0, directionalColor, &lightDir);
        t3d_light_set_count(1);

        /* Drawing flags for this geometry:
         *   SHADED    — interpolate vertex colors across triangles
         *   DEPTH     — write/test the depth buffer (correct occlusion)
         *   CULL_BACK — skip triangles whose normal faces away from camera.
         *               Winding order: CCW = front face (standard convention).
         *               This halves the RSP work for a closed mesh. */
        t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);

        /* Record the draw commands the first frame, replay every frame after.
         *
         * t3d_matrix_push uploads modelMatFP to the RSP matrix stack via DMA.
         * Even though the block is recorded once, the DMA reads the CURRENT
         * contents of modelMatFP each time the block runs — so the matrix
         * update above is always reflected. This works because modelMatFP is
         * in uncached memory (writes go straight to RDRAM, not a CPU cache). */
        if (!dplDraw) {
            rspq_block_begin();

            t3d_matrix_push(modelMatFP); /* DMA the model matrix → RSP stack   */
            t3d_vert_load(cubeVerts, 0, 8); /* DMA 8 vertices → RSP vertex cache  */
            t3d_matrix_pop(1);           /* pop matrix (vertices already transformed) */

            /* 12 triangles — 2 per face. All wound CCW when viewed from outside.
             *
             * Vertex index reference:
             *   0(-10,-10,-10) 1(+10,-10,-10) 2(+10,-10,+10) 3(-10,-10,+10)
             *   4(-10,+10,-10) 5(+10,+10,-10) 6(+10,+10,+10) 7(-10,+10,+10) */
            t3d_tri_draw(0, 5, 1); t3d_tri_draw(0, 4, 5); /* front  (z=-10) */
            t3d_tri_draw(2, 6, 7); t3d_tri_draw(2, 7, 3); /* back   (z=+10) */
            t3d_tri_draw(0, 3, 7); t3d_tri_draw(0, 7, 4); /* left   (x=-10) */
            t3d_tri_draw(1, 5, 6); t3d_tri_draw(1, 6, 2); /* right  (x=+10) */
            t3d_tri_draw(0, 1, 2); t3d_tri_draw(0, 2, 3); /* bottom (y=-10) */
            t3d_tri_draw(4, 7, 6); t3d_tri_draw(4, 6, 5); /* top    (y=+10) */

            /* Sync must be called after the last triangle batch. It tells the
             * system that the RSP is done writing triangle commands to the RDP,
             * so the RDP can safely rasterize them. Without this, the RDP might
             * start before the RSP has finished writing all commands. */
            t3d_tri_sync();

            dplDraw = rspq_block_end();
        }

        rspq_block_run(dplDraw);

        /* Flip to the completed framebuffer. This implicitly syncs — it waits
         * for the RDP to finish rasterizing before presenting, which in turn
         * means the RSP has also finished. Safe to overwrite modelMatFP on the
         * next iteration because the RSP is done with it by the time we return
         * here. */
        rdpq_detach_show();
    }

    t3d_destroy();
    return 0;
}
