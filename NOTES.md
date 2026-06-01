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

---

## Session 2 — From Cube to Game

The spinning cube became a real game skeleton: a scene framework, three playable
stages (Void, Star Field, Ice Moon), a 3D cockpit you look through, a positional
audio beacon, seeded procedural generation, and ghost persistence. The sections
below capture the concepts that earned their place in the code.

---

## The GLB Export Two-Step (the silent-failure trap)

Getting stage geometry out of Blender and into a `.t3dm` is two steps, and
**both failures are silent** — you get a valid-but-empty file, no error.

1. **Export with `export_materials='NONE'`.** Counterintuitively, exporting *no*
   materials is what *preserves* vertex colors. `'PLACEHOLDER'` and `'EXPORT'`
   suppress the `COLOR_0` attribute unless the Blender material explicitly wires
   a vertex-color node — which stage materials don't. `NONE` keeps the colors.

2. **Patch a dummy material index back in.** `gltf_to_t3d` *skips any primitive
   that has no material field* — silently producing a 65-byte empty model. So
   `patch_glb_material.py` injects `material=0` on every primitive before
   conversion.

Then convert with `--base-scale=1` for stage geometry. The default `--base-scale=64`
multiplies every coordinate by 64 before the int16 conversion, which **overflows**
for anything past ~512 units (512 × 64 = 32,768 > 32,767). Large stages need scale 1.

**Diagnosing:** `python inspect_glb.py file.glb` — if any primitive shows
`material=None`, the patch step didn't run.

**What surprised us:** "no materials" and "preserve colors" sound contradictory,
but the glTF exporter treats vertex colors as an *attribute* independent of the
material system. The material is only needed downstream so the converter doesn't
discard the geometry.

---

## The Cockpit Is a 3D Model, Not a 2D Overlay

The first cockpit was a flat 2D frame drawn over the scene. It looked pasted-on.
The real one is an actual 3D model floating a fixed distance in front of the
camera, re-oriented every frame so it tracks the view exactly.

The trick is building its model matrix **directly from the camera's basis
vectors** instead of from Euler angles. Given the look direction:

```
right = normalize(look × worldUp)
up    = right × look
```

Those three axes (right, up, −look) become the matrix's columns; the position
is `cameraPos + look × COCKPIT_DIST`. Because this is the *same* basis
`t3d_viewport_look_at` builds internally, the frame lines up perfectly at any
yaw/pitch. (Euler decomposition got the sign of one axis wrong at large angles —
the frame drifted. Basis vectors don't have that problem.) This math now lives
once in `camera.c`.

**Singularity:** when you look straight up, `right` is undefined (you can spin
freely about vertical). All flight clamps pitch to ±80°, but the helper still
guards the divide so a degenerate look can't produce a NaN or an FPU exception.

---

## Draw Order vs. Depth Test (why the HUD stopped clipping)

The cockpit poked *through* close objects (the Pioneer placeholder bled into the
frame). Cause: it was drawn with `T3D_FLAG_DEPTH`, so it depth-tested against the
world — and a near object won the test.

Fix: **drop the depth flag.** The cockpit is drawn *after* the whole 3D scene, so
without depth testing it simply paints on top — exactly what a fixed shell should
do. The lesson generalizes: anything that acts as a HUD shell (always-on-top,
fixed to the view) should render last *without* depth testing, not fight the
depth buffer.

---

## Positional Audio — Hearing in a Vacuum

Space has no sound, so the beacon is a *fiction with a rule*: the Pioneer's
electromagnetic residue, converted to audio (the same way NASA sonifies
magnetosphere "chorus"). The point is navigation by ear.

Pipeline: a synthesized looping `.wav64` plays on one mixer channel; every frame
we set its **pan** and **volume** from geometry:

- **Volume** = `300 / (dist + 10)`, clamped — a 1/distance falloff. Louder = closer.
- **Pan** = project the signal direction onto the camera's *right* axis (a dot
  product) → 0 = full left, 0.5 = centered, 1 = full right.

Audio must be serviced every frame (`mixer_try_play()` in the main loop) or it
stutters — so it lives at the top level, not inside a scene.

**The sound itself:** two tones a perfect fourth apart (168 + 224 Hz), each
drifting in pitch independently, under a "breathing" envelope made by multiplying
three slow sine waves of incommensurable rates. The result never repeats and
reads as alive rather than mechanical — which is the whole point.

---

## One Audio Output, One Owner (a bug we hit)

The N64 has exactly **one** audio output: the Audio Interface (AI) DMA's samples
from a small ring of RDRAM back buffers to the DAC, continuously. The whole job
is to keep those buffers full. But there are **two incompatible ways** to do it,
and they cannot coexist:

- **Raw audio** — you ask `audio_can_write()`, grab a buffer with
  `audio_write_begin()`, write samples in yourself, return it with
  `audio_write_end()`. *You* own the buffers.
- **The mixer** — you register sound sources as channels and call
  `mixer_try_play()` once a frame; the RSP mixer grabs the buffers, pulls from
  every channel, mixes (with pan/volume), and writes the combined result. *The
  mixer* owns the buffers.

Both reach for the same buffers and each assumes it's the sole owner. libdragon's
docs say it outright: you cannot mix-match them.

**What bit us:** the synth demo used raw audio. When the signal beacon was added,
`main.c` started pumping the *mixer* every frame, in every scene — including
while the synth was running. The two fought over the buffers: the synth rarely
got a free one, so its fill loop (which also advances the envelopes and the scope)
barely ran. Result: **no sound, frozen bars** — even though the preset text drew
fine, because that's just logic, not audio.

**The fix — commit to one owner.** We made the mixer the single audio path and
turned the synth into a mixer *waveform*: instead of shoving samples into the
hardware, it exposes a "give me N samples" callback (`samplebuffer_append` +
the existing oscillator code) that the mixer pulls from. Same sound math, new
plumbing. One owner, one `mixer_try_play()`, no collision.

**The rule to remember:** *one audio output means the whole program commits to
one way of feeding it.* On this project that's the mixer — every sound (beacon,
synth, future music/SFX) is a mixer channel. No exceptions, or they collide.

---

## Deterministic Generation — One Seed, One Run

The generative design rests on a single rule: **one `uint32_t` seed reproduces an
entire run.** Store the seed and you can replay the run; that's what makes compact
saves and ghost records possible.

For that to hold, every system draws from the same generator (`rng.h`, xorshift32)
in a **fixed, documented order**. Two consequences worth internalizing:

- **Draw order is a frozen contract.** `obstacles_generate` draws 7 values per
  obstacle in a set sequence. Reorder them and *every existing run changes*.
  Treat the order like an on-disk format.
- **Independent streams via domain XOR.** Two systems that shouldn't correlate
  (obstacles vs. gravity fields) seed from `seed` and `seed ^ constant`. Same run,
  uncorrelated sequences.

xorshift is chaotic: seed 1000 and 1001 produce unrelated runs. (Never seed with
0 — it's a dead state that outputs 0 forever; `rng_seed` remaps it to 1.)

---

## Architecture — Shared Systems vs. Scenes

The code splits into **scenes** (one screen each: a self-contained
init/update/draw triple) and **shared systems** the scenes reuse. When the same
logic appeared in two scenes, it got extracted:

- `rng` — the one PRNG (was copy-pasted into three files)
- `camera` — the camera-basis math (cockpit and signal both needed it)
- `flight` — the whole steer/thrust/drag/bounds model (Void and Star Field shared it)

**Triple-buffered matrices** recur everywhere: the RSP DMAs a matrix
asynchronously, so overwriting it next frame while the RSP still reads it would
corrupt the draw. Three `malloc_uncached` slots indexed by frame guarantee the
CPU and RSP never touch the same one. (Static, never-changing matrices can use a
single slot — there's nothing to race.)

**Scene registry as a dispatch table:** scenes live in an array indexed by the
`SCENE_*` constants, using designated initializers (`[SCENE_VOID] = {...}`) so
the index and the constant can't drift apart, with a `_Static_assert` that fails
the build if the table and the constant range disagree. Adding a scene is one
array row; the selector picks it up automatically.

**What surprised us:** the refactor made the ROM *smaller* — shared code is
compiled once instead of duplicated per scene.
