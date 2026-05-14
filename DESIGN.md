# The Colossus — N64 Rails Game
## Design Brief & Claude Code Context

---

## What This Project Is

A rails-based 3D space traversal game for real Nintendo 64 hardware, built with **libdragon** (preview branch) and **tiny3d**. The player travels through deep space on a fixed forward path — obstacle survival, not combat — searching for a mythic cosmic entity known as The Colossus. When found, the player becomes one.

Emotional touchstones: **Akira** (transformation as terrifying transcendence), **The Fountain** (lone traveler moving toward something sacred), **Passage by Jason Rohrer** (mortality, repetition, the finite journey — spirit not mechanics). Think meditative, inevitable, cosmic.

---

## Narrative

You are a seeker. Small, fast, propelled through the cosmos toward something incomprehensibly vast. The Colossus is out there — a being of such scale that worlds die in his wake. You don't know what you'll find. You don't know what you'll become.

The song that inspired this game asks: *"Do I dare to disturb the universe? Lest I become he the Colossus."* That tension — the cost of seeking, the fear of arrival — is the emotional core. **No actual song or lyrics are used in the game.**

---

## Three-Act Structure

### Act 1 — The Search
- Fast, almost frantic rails movement through deep space
- Player feels small and purposeful
- Environment: debris fields, dying stars, strange geometry
- Hints of the Colossus: his silhouette against a nebula, gravitational distortions, dead worlds in his wake
- Music: original, urgent, searching

### Act 2 — His Gravity
- Movement slows subtly — you're being pulled now, not just flying
- Rails begin to curve toward him
- Environment grows stranger, more sparse
- Encounters: frozen seekers who didn't make it — people, drifting, still reaching forward (see Ghost Mechanic)
- Music: sparser, the silence gets louder

### Act 3 — The Colossus
- You find him. He is enormous. The game slows to near stillness.
- The vessel falls away — the final approach is the person alone, unprotected, traveling as themselves
- No combat. You don't fight him. You approach.
- The player figure merges with the Colossus geometry — not violently, not triumphantly. Simply becoming part of something already ancient, already moving.
- The screen does not go black. The Colossus continues through space. Eternal or dead — you can't tell. You're part of it now.
- Music: crescendo, then something unresolved — not a conclusion

**CRITICAL — DO NOT RESOLVE THE AMBIGUITY:**
The ending must not answer whether becoming the Colossus is death or transcendence, triumph or tragedy. This is intentional and load-bearing to the entire experience. Does the Colossus even know if it's alive or dead? Does scale make that distinction meaningless? The transformation sequence should make the question feel unanswerable — and somehow okay. Visually, tonally, and in code: hold it open. Never tip toward triumph. Never tip toward tragedy. Just vastness.

---

## Core Gameplay

**Genre:** Rails traversal / obstacle survival (Star Fox 64 locomotion, not Star Fox 64 combat)

**Player character:** A person — a human figure traveling through space. Not a pilot in a cockpit, not a ship. Small, exposed, vulnerable. This creates identification not distance. In Acts 1 and 2 they travel within some kind of vessel; in Act 3 the vessel falls away and they continue alone.

**The loop:**
- Player moves forward automatically on a fixed rail through 3D space
- Analog stick steers left/right/up/down within a bounded play area
- Obstacles must be avoided, not destroyed
- Survival = staying alive long enough to reach The Colossus

**Obstacle philosophy — the universe is indifferent, not hostile:**
- Gravitational fields that pull the player off course — player fights the drift
- Debris from dead worlds the Colossus has already passed through
- Ghost seekers (see Ghost Mechanic) — environmental storytelling + possible hazard
- Cosmic phenomena: pulsar waves, collapsing stars — timed movement windows
- Increasing exposure/degradation as you approach — the journey itself consumes you

**Pacing rhythm:**
- Flow → disruption → flow again
- Early: chaotic, fast
- Mid: sparse, strange, dreamlike
- Late: obstacles fall away, you're just moving toward him

**No shooting mechanic.** No score. Life depletes on obstacle contact — death is possible and meaningful.

---

## Ghost Mechanic

When the player dies, their position along the rail, their lateral/vertical offset, and their run index are written to **SRAM** on cartridge.

On subsequent runs, those positions are loaded and rendered as frozen figures — people, drifting in the exact spot and posture they died, still reaching forward. They become part of the environment of Act 2, growing denser closer to The Colossus.

- They are the player's own death history made visible
- They are also environmental storytelling — other seekers who came before
- Their effect on gameplay (purely cosmetic vs soft hazard) TBD — iterate
- Cap stored deaths at a reasonable number given SRAM limits (TBD, ~5–10)
- Ghost figures should feel still and sad, not threatening

---

## Visual Direction

**Aesthetic:** Lo-fi N64 polygon style embraced fully — not apologized for. The hardware constraints *are* the art direction. Think how 2001: A Space Odyssey feels sparse and overwhelming — that's the target emotion, achieved through simplicity not detail.

- Deep space: stark star fields, shifting color temperature as player approaches Colossus
- The Colossus rendered as simple but massive geometry — overwhelming by scale, not complexity
- Player character: a small human figure — minimal geometry, recognizably a person
- Color palette: deep blacks, cold blues and purples in Act 1 → warm cosmic oranges and whites in Act 3
- tiny3d's GLTF pipeline for models (via Blender + Fast64 materials)

**Reference:** StarStrike 64 proved a rails shooter is achievable in libdragon/tiny3d. Study its approach.

---

## Audio Direction

**No licensed music is used.** All audio is original.

- Ambient generative-feeling soundtrack that shifts with game state
- libdragon supports **Opus audio compression** for compressed music streaming
- Sound design prioritizes space, silence, and scale — not action game energy
- The transformation sequence in Act 3 is the audio climax

---

## Technical Stack

| Component | Tool |
|-----------|------|
| SDK | libdragon (preview branch) |
| 3D rendering | tiny3d |
| 3D modeling | Blender + Fast64 materials → GLTF |
| Emulator | Ares (Homebrew mode on) — primary dev target |
| Secondary emulator | Gopher64 (performance testing) |
| Build system | Make + Docker toolchain (libdragon official image) |
| Language | C (primary), with tiny3d C API |
| Audio | libdragon audio + Opus for music streaming |

**tiny3d notes:**
- Requires libdragon **preview branch** specifically
- Models and matrices are DMA'd into RSP each frame — keep both in RDRAM while rendering
- Use GLTF import pipeline with Fast64 custom properties enabled
- Interops directly with libdragon's RDPQ API

**Pyrite64:** Watch this project — it's a visual editor + toolchain manager built on libdragon/tiny3d by the same author (HailToDodongo). Currently WIP/early dev but may become useful for scene authoring.

---

## Project Priorities

1. Get a basic rails movement loop working first — player figure moving forward, analog stick steering, collision with a simple obstacle
2. Establish the visual tone early — star field, camera, player character model
3. Add ghost mechanic — SRAM write on death, load and render frozen figures on next run
4. Add obstacle variety incrementally
5. Audio last, or in parallel once gameplay loop is stable

---

## What Claude Code Should Know

- This is a **side project** with limited time — weekdays light, weekends lighter (family commitments)
- Prefer **simple, working, incremental** code over ambitious incomplete systems
- Always explain N64-specific constraints when they affect implementation choices
- Keep the emotional vision in mind: **meditative, inevitable, cosmic** — not an action game
- When in doubt, do less and do it well

---

## Generative Systems

The game uses procedural and generative approaches so no two runs are identical and the universe feels organic and alive. These are rules the artist sets; the system takes over from there.

### 1. Seeded Run Generation (Foundation)
Every run derives a single `uint32_t` seed — read from the N64 hardware timer (`TICKS_READ()`) at the moment the run begins, or derived from the SRAM run count. Pass this seed into a fast pseudo-random number generator (LCG or xorshift32 — both are trivial to implement in C and cheap on the CPU). Every system below that needs randomness draws from this seeded PRNG, so the entire run is deterministic from that single value. If the seed were stored, a run could be replayed exactly. Everything else builds on this.

```c
// xorshift32 — fast, good enough distribution
uint32_t rng_state;
uint32_t rng_next() {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}
```

### 2. Value Noise for Environment
Use value noise (simpler than Perlin, sufficient for this use case) to drive star field density, nebula color cloud placement, and debris clustering. Sample a 2D noise function seeded per run. High noise values = denser debris or brighter nebula region. Low values = empty void. This gives space an organic, non-uniform feeling without hand-placing anything. The noise function itself is just a seeded hash over a grid with smoothed interpolation between grid points — implementable in ~20 lines of C.

### 3. Generative Music via Phase-Offset Loops
Compose 4–6 short ambient audio loops, each a different length (e.g. 7s, 11s, 13s, 17s, 19s, 23s — prime lengths prevent repetition). Play all simultaneously via libdragon's audio mixer. Because their lengths are incommensurable, the combination drifts continuously and never exactly repeats within any practical timeframe. Control the emotional state by fading individual loop volumes in and out based on game state — Act 1 layers are urgent and sparse, Act 2 layers are slower and deeper, Act 3 layers are whatever serves the ambiguity. ~30 seconds of composed audio yields an effectively infinite score.

### 4. Vertex Displacement on the Colossus
Each frame, displace each vertex of the Colossus mesh along its normal by a sum of sine waves:

```c
float displacement = 
    0.3f * sinf(vertex.x * 0.8f + time * 0.4f) +
    0.2f * sinf(vertex.y * 1.1f + time * 0.27f) +
    0.1f * sinf(vertex.z * 1.4f + time * 0.13f);
vertex.pos += vertex.normal * displacement;
```

Two or three sine waves of different spatial and temporal frequencies produce complex, organic, non-repeating surface motion. He breathes. He does not pulse with hostility — he simply exists, vast and slow. Keep displacement amplitude small relative to his scale.

### 5. Ghost Figure Degradation
Each ghost record in SRAM stores a `run_index` — which run the player died on. When rendering a ghost, use that index to scale a noise-based vertex offset:

```c
float age_factor = (float)(current_run - ghost.run_index) / MAX_RUNS;
float drift = age_factor * 2.0f * sinf(vertex.id * 17.3f); // deterministic per vertex
vertex.pos += drift;
```

Older ghosts dissolve — their geometry drifts toward illegibility. The oldest seekers are barely recognizable shapes. Memory fading as geometry.

### 6. Proximity-Reactive Particles
Each particle has a base drift vector (set at spawn from seeded PRNG) and a pull vector (directed toward the Colossus position). Blend between them based on player distance to Colossus:

```c
float t = 1.0f - clamp(distance / MAX_DISTANCE, 0.0f, 1.0f);
particle.velocity = lerp(particle.base_drift, particle.pull_toward_colossus, t);
```

At full distance: pure random drift, cold color. At close range: pulled toward him, warm color. His gravity is felt before he is seen.

### 7. Colossus Evolution Across Runs — OPEN, DO NOT IMPLEMENT YET
A small float stored in SRAM accumulates across runs tracking the player's collective journey history. The Colossus changes subtly in response. **How he changes is not yet decided.** Possibilities being held open:
- He becomes more defined — as if being perceived reveals him
- He becomes more vast — scale asserting itself gradually
- He becomes more still — more glacial, more ancient
- He becomes less readable — drifting toward something less humanoid, more cosmic
- He does not change at all — the ghosts accumulate around him, the change is always in the seeker

Do not resolve this creatively or in code until the direction feels right.

---

## Star Field

A hybrid of three layers creating the illusion of infinite depth at N64 cost.

### Layer 1 — Background RDP Star Carpet
Drawn directly to the framebuffer before any 3D rendering. Hundreds of points and tiny quads at varied brightness, positioned by run seed. Essentially free — the RDP handles this before tiny3d touches anything.

### Layer 2 — Midground Billboard Sprites
A handful of larger star clusters or nebula wisps as flat sprites that always face the camera. Slight parallax shift based on player lateral movement creates a powerful illusion of depth. These give the space atmosphere.

### Layer 3 — Foreground 3D Geometry Stars
10–20 actual vertex stars — simple shapes (crosses, small tetrahedra) in true 3D space. The player flies near or through them. Real parallax, real presence. Some may serve as subtle spatial landmarks.

**Depth through brightness:** Background stars seeded dim, foreground bright. Enormous perceived depth from simple rules.

**As the player approaches the Colossus:**
- Color temperature shifts — cold blue-white → warm orange-white
- Geometry stars drift subtly toward him — space feels like it's leaning
- Star density shifts — Act 1 dense and chaotic, Act 2 sparse, Act 3 almost empty

**Ghost figures here:** Frozen in true 3D space like geometry stars. Real parallax, real presence. Completely still inside flowing space. That stillness will be haunting.

---

## Rendering Techniques (from Star Fox 64 source study)

These patterns come from studying the Star Fox 64 recompilation codebase. They are proven N64 techniques worth applying.

### Frame-Count Flicker for Ghosts

Alternate a lighting or color value every other frame using `gGameFrameCount % 2`. No texture animation, no extra geometry — a free shimmer effect that makes ghost figures feel unstable and eerie. Ghosts should feel like they are almost not there. The flicker reinforces that.

```c
// In ghost draw — dim the ambient every other frame
uint8_t flicker_amb = (frame_count % 2 == 0) ? 60 : 40;
t3d_light_set_ambient((uint8_t[]){flicker_amb, flicker_amb + 20, 180, 0xFF});
```

### Color Table Particle Aging

Store color+alpha at each particle age stage in a lookup table. As a particle ages, increment an index into the table. Fire: orange → yellow → white → transparent. Debris: solid → fading. No per-particle color math at runtime — just a table lookup per frame.

```c
static const uint8_t smoke_colors[][4] = {
    {255, 120,  40, 220},  /* young — orange */
    {255, 200,  80, 160},  /* mid   — yellow */
    {255, 255, 180,  80},  /* old   — white  */
    {200, 200, 200,  20},  /* dying — grey   */
};
uint8_t *col = smoke_colors[particle.age_index];
```

### Contrail / Trail via Z-Scaled Mesh

Stretch a simple sphere or quad along the Z axis to fake a motion trail without trail geometry. `Matrix_Scale(1.0f, 1.0f, trail_length)` on a ball mesh creates a convincing teardrop contrail. Apply this to proximity-reactive particles near the Colossus — as they get pulled in, stretch them along their velocity vector.

### Starfield Density — 3000 Stars

Star Fox 64 uses 3000 stars scattered across screen space with a seeded LCG, rendered as 1×1 pixel triangles via ortho projection. Our current starfield uses 150 world-space geometry stars which gives real parallax but lower density. The two approaches can coexist: their method for a dense backdrop carpet, ours for foreground presence stars.

---

## Design Philosophy — Questions This Game Does Not Answer

These are open intentionally. Do not resolve them in code, visuals, audio, or text:

- Is becoming the Colossus death or transcendence?
- Is the Colossus alive or dead — does it even know?
- Are the frozen seekers who didn't reach him failures, or did they arrive differently?
- Does scale make the distinction between life and death meaningless?
- How does the Colossus change across runs, and what does that change mean?

The player should carry these questions out of the game with them. Any implementation decision that accidentally answers one of them should be reconsidered. The game's job is to make the questions feel vast and okay, not to resolve them.
