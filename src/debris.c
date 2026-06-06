#include <libdragon.h>
#include <math.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "debris.h"
#include "rng.h"

/* Tier sizes. Total stays small enough that a full per-frame distance scan is
 * trivial, and only the handful within RENDER_RANGE actually draw. */
#define N_LARGE      6
#define N_SCATTER   90
#define N_GRADIENT 160
#define N_TOTAL    (N_LARGE + N_SCATTER + N_GRADIENT)

/* The three Blender-authored debris models, loaded once. */
#define M_ASTEROID 0
#define M_SHARD    1
#define M_WRECK    2
#define N_MODELS   3

/* Each model is normalized to a 1-Blender-unit radius and converted at the
 * default --base-scale=64, so its t3dm radius is ~64 units. A piece's matrix
 * scale is therefore (desired game radius / 64). */
#define MODEL_UNIT  64.0f

/* Cull radius. Matches the scene far-clip (8000): anything past it is clipped
 * by the GPU anyway, so we skip building its draw entirely. */
#define RENDER_RANGE  8000.0f

/* Uniform-scatter bounds. Kept inside ~32000 so the s16.16 model-matrix
 * translation term never overflows (see scene_void.c). */
#define SC_X      13000.0f
#define SC_Y       5000.0f
#define SC_ZNEAR   1000.0f
#define SC_ZFAR  -27000.0f

/* Gradient cluster around the Pioneer. d = GRAD_RMAX * u^GRAD_POW with u
 * uniform in [0,1]: POW > 1 pulls most pieces close to the source, leaving a
 * thinning tail that still reaches out toward the spawn region. */
#define GRAD_RMAX  16000.0f
#define GRAD_POW       2.2f

static float      px[N_TOTAL], py[N_TOTAL], pz[N_TOTAL];
static uint8_t    model[N_TOTAL];        /* which mesh (M_*) each piece uses */
static T3DMat4FP *mats;                  /* one static world matrix per piece */
static T3DModel  *models[N_MODELS];
static bool       ready = false;

static float frand(Rng *r, float lo, float hi) {
    return lo + rng_unit(r) * (hi - lo);
}

/* Build piece i's static transform: uniform scale + random Euler rotation at
 * its (already-set) world position. Written once; never rewritten, so a single
 * (non-triple-buffered) matrix is safe — there is no async-overwrite race. */
static void build_mat(int i, float gameRadius, Rng *r) {
    float s = gameRadius / MODEL_UNIT;
    float scale[3] = { s, s, s };
    float rot[3]   = { frand(r, 0.0f, 6.2831853f),
                       frand(r, 0.0f, 6.2831853f),
                       frand(r, 0.0f, 6.2831853f) };
    float pos[3]   = { px[i], py[i], pz[i] };
    t3d_mat4fp_from_srt_euler(&mats[i], scale, rot, pos);
}

static void clampf(float *v, float lo, float hi) {
    if (*v < lo) *v = lo;
    if (*v > hi) *v = hi;
}

void debris_init(uint32_t seed, float px0, float py0, float pz0) {
    if (!ready) {
        mats = malloc_uncached(sizeof(T3DMat4FP) * N_TOTAL);
        models[M_ASTEROID] = t3d_model_load("rom:/asteroid.t3dm");
        models[M_SHARD]    = t3d_model_load("rom:/shard.t3dm");
        models[M_WRECK]    = t3d_model_load("rom:/wreck.t3dm");
        ready = true;
    }

    Rng r;
    rng_seed(&r, seed);
    int i = 0;

    /* Tier 1 — large landmarks: big chunks scattered across the void as fixed
     * reference points the player can pick out and steer toward. Out here it is
     * mostly natural rock and ice — no wreckage this far from the source. */
    for (int n = 0; n < N_LARGE; n++, i++) {
        px[i] = frand(&r, -SC_X, SC_X);
        py[i] = frand(&r, -SC_Y, SC_Y);
        pz[i] = frand(&r, SC_ZFAR, SC_ZNEAR);
        model[i] = (rng_next(&r) & 1) ? M_SHARD : M_ASTEROID;
        build_mat(i, frand(&r, 260.0f, 520.0f), &r);
    }

    /* Tier 2 — uniform scatter: a thin baseline everywhere so there is always
     * something nearby to orient against, no matter where the player drifts. */
    for (int n = 0; n < N_SCATTER; n++, i++) {
        px[i] = frand(&r, -SC_X, SC_X);
        py[i] = frand(&r, -SC_Y, SC_Y);
        pz[i] = frand(&r, SC_ZFAR, SC_ZNEAR);
        model[i] = (rng_next(&r) & 1) ? M_SHARD : M_ASTEROID;
        build_mat(i, frand(&r, 18.0f, 70.0f), &r);
    }

    /* Tier 3 — gradient cluster: density climbs toward the Pioneer, AND the mix
     * shifts to wreckage. A random direction on the unit sphere times a
     * small-biased distance packs most pieces near the source; the closer you
     * get, the more the field becomes the broken ships of those who came before.
     * Following the thickening wreckage leads inward, into the signal's range. */
    for (int n = 0; n < N_GRADIENT; n++, i++) {
        float d  = GRAD_RMAX * powf(rng_unit(&r), GRAD_POW);
        float th = frand(&r, 0.0f, 6.2831853f);
        float ct = frand(&r, -1.0f, 1.0f);          /* cos(polar) for an even sphere */
        float st = sqrtf(1.0f - ct * ct);
        px[i] = px0 + st * cosf(th) * d;
        py[i] = py0 + ct * d;
        pz[i] = pz0 + st * sinf(th) * d;
        clampf(&px[i], -SC_X, SC_X);
        clampf(&py[i], -SC_Y, SC_Y);
        clampf(&pz[i], SC_ZFAR, SC_ZNEAR);

        float u = rng_unit(&r);                     /* wreck-weighted mix */
        model[i] = (u < 0.55f) ? M_WRECK : (u < 0.85f) ? M_ASTEROID : M_SHARD;
        build_mat(i, frand(&r, 14.0f, 64.0f), &r);
    }
}

void debris_draw(float camX, float camY, float camZ) {
    if (!ready) return;

    /* Cold, dim side-light: the chunks read as dark rock/metal catching a little
     * starlight, never competing with the warm Pioneer or the colored backdrop. */
    uint8_t amb[4] = { 26, 28, 38, 0xFF };
    uint8_t dir[4] = { 150, 158, 178, 0xFF };
    T3DVec3 ld = {{ 0.4f, 0.7f, 0.3f }};
    t3d_vec3_norm(&ld);
    t3d_light_set_ambient(amb);
    t3d_light_set_directional(0, dir, &ld);
    t3d_light_set_count(1);
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_DEPTH | T3D_FLAG_CULL_BACK);

    float range2 = RENDER_RANGE * RENDER_RANGE;
    for (int i = 0; i < N_TOTAL; i++) {
        float dx = px[i] - camX, dy = py[i] - camY, dz = pz[i] - camZ;
        if (dx*dx + dy*dy + dz*dz > range2) continue;   /* beyond far-clip — skip */
        t3d_matrix_push(&mats[i]);
        t3d_model_draw(models[model[i]]);
        t3d_matrix_pop(1);
    }
}
