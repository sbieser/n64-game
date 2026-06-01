#pragma once
#include <t3d/t3d.h>

/*
 * shapes.h — five hand-built obstacle meshes.
 *
 * These are the reusable "rock and ice debris" primitives: vertex data written
 * by hand (no model files), each ~5-8 vertices, vertex-colored, wound CCW for
 * back-face culling.  Used by the rails obstacle field and as placeholders
 * elsewhere.  See shapes.c for the per-shape vertex layout diagrams.
 *
 * USAGE
 *   shapes_init();              // once at startup — allocates + fills all buffers
 *   ...set combiner + draw flags + push a model matrix...
 *   draw_shape(SHAPE_OCTA);     // emits the triangles for one shape
 */

#define SHAPE_CUBE    0
#define SHAPE_TETRA   1
#define SHAPE_OCTA    2
#define SHAPE_PYRAMID 3
#define SHAPE_PRISM   4

/* Allocate and fill every shape's uncached vertex buffer. Idempotent (a second
 * call is a no-op), so multiple scenes can safely call it on entry. */
void shapes_init(void);

/* Draw one shape (a SHAPE_* constant) at the current matrix-stack transform.
 * The caller sets combiner, draw flags, and pushes the model matrix first. */
void draw_shape(int shape);
