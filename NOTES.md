# N64 Development Notes

Running log of concepts, gotchas, and hardware details uncovered while building this project.

---

## Session 1 — Spinning Cube (first working ROM)

Got a rainbow-shaded, back-face-culled, depth-buffered spinning cube running at
60 FPS in Ares. Built from raw vertex data — no model files, no asset pipeline.

**What the rendered output tells us:**
- Vertex color interpolation works — the gradient across each face is the GPU
  blending corner colors across the triangle surface (Gouraud shading)
- Lighting is subtle but present — the directional light creates a warm/cool
  variation depending on which face is angled toward it
- Back-face culling is correct — the cube reads as a solid object, not a
  wireframe or inside-out shape
- 60 FPS with this much headroom means we can add significant geometry and
  logic before performance becomes a concern

**Things that surprised us:**
- The RSP has NO floating point unit. All vertex math is s16.16 fixed-point.
  The CPU does float → `t3d_mat4_to_fixed()` → RSP gets fixed-point data.
  Two entirely different compute models sharing one frame.
- `malloc_uncached` appearing in game code is a hardware constraint, not
  style. It's the DMA boundary between CPU cache and RSP RDRAM access made
  visible in C.
- The display list records an *address reference*, not data. The RSP DMA's
  fresh matrix contents on every replay — so updating `modelMatFP` before
  `rspq_block_run` is all that's needed to animate the cube.
- The concurrent pipeline (CPU + RSP + RDP all running simultaneously) that
  the N64 shipped with in 1996 is structurally identical to modern explicit
  GPU APIs (Vulkan command buffers, Metal command encoders, DX12 command
  lists). The N64 just made the seams visible.

---

## The Hardware Pipeline

```
CPU (MIPS R4300i, 93 MHz)
  |
  | builds a list of commands (the "display list")
  v
RSP (Reality Signal Processor, 62 MHz)
  |  runs tiny3d's microcode — transforms vertices, applies lighting,
  |  clips triangles, emits triangle commands to the RDP
  v
RDP (Reality Display Processor)
  |  rasterizes triangles: fills pixels, applies textures, blends colors,
  |  writes to the framebuffer in RDRAM
  v
Framebuffer (RDRAM, 4.5 MB total)
  |
  v
Video Interface → TV
```

The CPU, RSP, and RDP all run **concurrently**. The CPU never waits idle — while the RSP processes frame N, the CPU is building commands for frame N+1.

---

## The Display List / RSPQ System

The CPU does not talk to the RSP one command at a time. It writes commands into a ring buffer (the RSPQ), then signals the RSP. The RSP drains the buffer asynchronously.

`rspq_block_begin()` / `rspq_block_end()` record a fixed sequence into a reusable block. `rspq_block_run()` replays it. For static geometry (a cube that never changes shape), record once and replay every frame — no per-frame CPU overhead for the geometry submission itself.

---

## Triple Buffering

libdragon allocates 3 framebuffers in RDRAM:

```
Frame N:   TV displaying A | RDP drawing into B | CPU building commands for C
Frame N+1: TV displaying B | RDP drawing into C | CPU building commands for A
```

`display_get()` always returns the next available buffer. `rdpq_detach_show()` waits for the RDP to finish the current buffer, then presents it.

---

## Uncached Memory (`malloc_uncached`)

The N64 CPU has a write-back cache. Writes go to the cache first and may not reach RDRAM immediately. The RSP's DMA engine reads directly from RDRAM — it bypasses the cache entirely.

**Problem:** CPU writes matrix to cached address → cache not flushed → RSP DMA reads stale RDRAM data → wrong matrix applied.

**Solution:** Allocate vertex buffers and matrices with `malloc_uncached()`. Writes go straight to RDRAM. The RSP always sees fresh data.

Rule of thumb: anything the RSP or RDP will DMA from — vertex data, matrices, index buffers, display lists — must be in uncached memory.

---

## The RSP Vertex Cache

The RSP has 4 KB of DMEM (data memory, on-chip). Of that, tiny3d reserves space for **70 vertices** in the vertex cache. `t3d_vert_load()` DMA's vertex data from RDRAM into those slots.

Vertices are stored as `T3DVertPacked` — two vertices per 32-byte struct. This packing aligns each pair to a cache line boundary, making the DMA maximally efficient.

After `t3d_vert_load()`, the loaded vertices stay in the cache until the next load. `t3d_tri_draw(a, b, c)` references vertices by their slot index (0–69). No re-uploading needed until you exceed 70 vertices or want different ones.

---

## The Matrix Pipeline

Every vertex goes through three spaces:

```
Model space   — where the mesh is defined (cube centered at origin)
    × model matrix (position, rotation, scale of this object)
World space   — where objects live relative to each other
    × view matrix (inverse of camera transform)
View/clip space — what the camera sees
    × projection matrix (perspective divide, maps to screen)
Screen space  — final 2D pixel coordinates + depth
```

tiny3d keeps a matrix stack on the RSP. `t3d_matrix_push()` multiplies the new matrix with the top of the stack and pushes the result. `t3d_matrix_pop()` removes the top. The viewport's projection and view matrices are set separately via `t3d_viewport_attach()`.

Matrices are in **s16.16 fixed-point** format (`T3DMat4FP`) on the RSP — 16-bit integer part, 16-bit fractional part. The CPU works in float (`T3DMat4`) and converts via `t3d_mat4_to_fixed()`.

---

## Back-Face Culling and Winding Order

`T3D_FLAG_CULL_BACK` tells the RSP to skip triangles whose surface faces away from the camera. For a closed mesh like a cube, you never see the inside, so this halves the RSP workload for free.

The RSP determines facing by **winding order**: when viewed from the front, a front face's vertices go **counter-clockwise (CCW)**. If they go clockwise, it's a back face and gets discarded.

Verified cube winding (CCW when viewed from outside each face):
- Front  (z=−10): (0,1,5) (0,5,4)
- Back   (z=+10): (2,6,7) (2,7,3)
- Left   (x=−10): (0,3,7) (0,7,4)
- Right  (x=+10): (1,5,6) (1,6,2)
- Bottom (y=−10): (0,1,2) (0,2,3)
- Top    (y=+10): (4,7,6) (4,6,5)

---

## Lighting

tiny3d supports:
- **Ambient** — always-on constant color added to every vertex
- **Directional** — infinite-distance light (like the sun); intensity = dot(normal, lightDir)
- **Point** — positional light; up to 7 total (directional + point combined)

Lighting is **per-vertex**, computed on the RSP when vertices are loaded. Pixel-level shading (normal maps, etc.) is not available — the N64 has no fragment shader equivalent.

Directional lights must be re-applied after each `t3d_viewport_attach()` call because the RSP transforms light directions into **view space** using the current view matrix. If you call `t3d_viewport_attach()` again (e.g. for a second viewport), the view matrix changes and the lights need updating.

---

## Asset Pipeline (future reference)

When we add models:

```
Blender → export .glb → gltf_to_t3d → .t3dm (in filesystem/)
                      → mksprite for textures (in filesystem/)
```

Files in `filesystem/` get packed into a DFS (Dragon File System) archive embedded in the ROM. At runtime, open them with:

```c
t3d_model_load("rom:/mymodel.t3dm");
```

The `rom://` prefix routes through libdragon's virtual filesystem to the DFS.
