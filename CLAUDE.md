# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 3D Nintendo 64 homebrew game built directly on libdragon + tiny3d. No pyrite64 engine — all code is hand-written C against the raw libraries.

Scott does not write code. Claude writes everything. Scott directs. Always explain what code does and why — the goal is learning, not just working output. Always plan before coding.

## Build

Builds run inside **MSYS2 bash** (`C:\msys64\usr\bin\bash.exe`). The SDK lives at `/pyrite64-sdk` inside MSYS2 (= `C:\msys64\pyrite64-sdk` on Windows).

```bash
# From MSYS2 bash, in the project directory
cd /c/dev/n64
make          # build game.z64
make clean    # clean build artifacts
```

To run after building, launch Ares manually: `C:\ares-v147\ares.exe` and load `game.z64`.

## SDK Layout

| Path (MSYS2) | Contents |
|---|---|
| `/pyrite64-sdk/bin/` | Tools: `make`, `mips64-elf-gcc`, `gltf_to_t3d`, `mkdfs`, `n64tool`, `mksprite`, `audioconv64` |
| `/pyrite64-sdk/mips64-elf/lib/` | `libdragon.a`, `libdragonsys.a`, `libt3d.a` |
| `/pyrite64-sdk/mips64-elf/include/` | `libdragon.h` and all libdragon headers |
| `/pyrite64-sdk/mips64-elf/include/t3d/` | tiny3d headers: `t3d.h`, `t3dmath.h`, `t3dmodel.h`, `t3dskeleton.h`, `t3danim.h`, `tpx.h` |
| `/pyrite64-sdk/include/n64.mk` | Core build system include |
| `/pyrite64-sdk/include/t3d.mk` | Prepends `-lt3d` to linker flags, defines `T3D_GLTF_TO_3D` |

## Makefile Pattern

```makefile
export N64_INST = /pyrite64-sdk
include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk
```

`n64.mk` defines all compiler flags, tool paths, and pattern rules. The `.z64` pattern rule chains: `.c` → `.o` → `.elf` → `.z64`. A `.dfs` (Dragon File System archive) can optionally be embedded in the ROM for runtime asset loading.

## Project Structure

```
C:\dev\n64\
├── Makefile          # build system
├── CLAUDE.md         # this file
├── NOTES.md          # N64 concepts and learnings
├── src/
│   └── main.c        # entry point and game loop
└── filesystem/       # contents become the ROM's DFS (rom:// prefix at runtime)
```

Assets added later:
```
├── assets/           # source assets (.glb models etc.) before conversion
└── filesystem/       # converted assets (.t3dm, .sprite, etc.) loaded at runtime
```

## N64 + tiny3d Architecture

### The Hardware Pipeline

```
CPU (MIPS R4300) — runs your C code
  ↓ rspq display list (queued commands)
RSP (Reality Signal Processor) — tiny3d runs here; transforms vertices, applies lighting
  ↓ triangle commands
RDP (Reality Display Processor) — rasterizes triangles, applies textures, writes pixels
  ↓
Framebuffer (RDRAM) → TV output
```

### Frame Structure

Every frame follows this pattern:

```c
rdpq_attach(display_get(), display_get_zbuf()); // target the current framebuffer
t3d_frame_start();                              // reset RSP state
t3d_viewport_attach(&viewport);                 // load camera/projection matrices
t3d_screen_clear_color(...);
t3d_screen_clear_depth();
// ... draw calls ...
rdpq_detach_show();                             // flip to screen
```

### Triple Buffering and Matrix Safety

libdragon uses 3 framebuffers. While the TV displays buffer A, the CPU writes buffer B, and the RSP may still be reading buffer C. Matrices passed to `t3d_matrix_push()` are DMA'd by the RSP asynchronously — the CPU must not overwrite them until the RSP is done. Solution: allocate one matrix per framebuffer via `malloc_uncached()` and index by frame.

```c
T3DMat4FP *modelMats = malloc_uncached(sizeof(T3DMat4FP) * 3);
// each frame: write to modelMats[display_get_index()]
```

### Display Lists (rspq blocks)

Static geometry can be recorded once and replayed every frame:

```c
rspq_block_t *block = NULL;
if (!block) {
    rspq_block_begin();
    // ... t3d draw calls ...
    block = rspq_block_end();
}
rspq_block_run(block); // replay each frame
```

### Vertex Format

```c
// T3DVertPacked holds TWO vertices (interleaved for RSP DMA efficiency)
T3DVertPacked verts[N]; // N = ceil(vertex_count / 2)

verts[0] = (T3DVertPacked){
    .posA = {x, y, z},           // s16, world units
    .normA = t3d_vert_pack_normal(&normalVec), // 5.6.5 packed
    .rgbaA = 0xRRGGBBAA,
    .stA   = {u, v},             // 10.5 fixed-point pixel coords
    // posB, normB, rgbaB, stB for second vertex
};

t3d_vert_load(verts, 0, vertexCount); // count = individual vertices (not packed structs)
t3d_tri_draw(0, 1, 2);               // index into vertex cache (0-69)
t3d_tri_sync();                       // required after last triangle batch
```

Vertices must be in `malloc_uncached()` memory before passing to `t3d_vert_load`.

### Key Draw Flags

```c
t3d_state_set_drawflags(
    T3D_FLAG_DEPTH      |  // enable depth buffer (Z-test)
    T3D_FLAG_SHADED     |  // use vertex colors
    T3D_FLAG_CULL_BACK     // cull back faces (CCW = front)
);
// T3D_FLAG_NO_LIGHT disables normal-based lighting, uses raw vertex color
```

### Lighting

```c
t3d_light_set_ambient(colorRGBA8);                // always active
t3d_light_set_directional(0, colorRGBA8, &dir);  // up to 7 directional or point lights
t3d_light_set_count(1);                           // how many non-ambient lights
// directional lights must be re-set after each t3d_viewport_attach (they depend on view matrix)
```

## Reference Repos (local)

| Repo | Path |
|---|---|
| libdragon source | `C:\repos\libdragon` |
| tiny3d source + 25 examples | `C:\repos\tiny3d` |
| pyrite64 (reference only) | `C:\repos\pyrite64` |

The tiny3d examples at `C:\repos\tiny3d\examples\` are the best reference for API usage. Start with `00_quad\main.c` for manual geometry, `01_model\main.c` for model loading.

## External Reference

https://n64squid.com/homebrew/libdragon/
