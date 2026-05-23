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
- Hints of the Colossus: his silhouette against a nebula, gravitational distortions, new stars forming in his wake
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

**Player character:** A person — a seeker traveling through deep space. The game is played from inside the cockpit: the player looks out through a wide horizontal windshield at the void, with a dark instrument dashboard below. The cockpit is a 2D rdpq overlay drawn after the 3D scene each frame — essentially free to render. At arrival, the cockpit fades away: instruments go dark, the frame dissolves, the hand is gone. The player becomes a bare point of view looking at something incomprehensible. The intimacy of the cockpit makes its removal the game's most significant visual moment. See `planning/colossus/visual/cockpit.md` for the full visual language.

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

When the player dies, their position and run index are written to **SRAM** on cartridge (32 KB cartridge SRAM). Stage unlock state fits in a two-byte bitmask. Ghost pools for all stages fit comfortably within the 32 KB limit. Compatible with SummerCart 64, EverDrive, and all major emulators.

On subsequent runs, those positions are loaded and rendered as frozen figures — people, drifting in the exact spot and posture they died, still reaching forward. They become part of the environment of Act 2, growing denser closer to The Colossus.

- They are the player's own death history made visible
- They are also environmental storytelling — other seekers who came before
- Their effect on gameplay (purely cosmetic vs soft hazard) TBD — iterate
- Cap: 8 ghosts (current implementation); oldest evicted when full
- Ghost figures should feel still and sad, not threatening

---

## Obstacle Variety

All obstacles share one philosophy: **the universe is indifferent, not hostile.** Nothing is trying to kill the player. Things simply exist, and the player must navigate around them. Each obstacle type should feel like a natural phenomenon with its own logic, not a game designer placing enemies.

Implementation note: obstacles are generated from the run seed, so each phenomenon's position and parameters come from `xorshift32` draws. The mechanic code lives in `obstacles.c`; the visual rendering uses existing shape primitives plus new quad/ring geometry.

---

### Debris Field *(planned — Act 1)*
Dense clusters of rotating geometry shapes — rock and ice, the scattered remnants of worlds the Colossus has passed through. The Colossus didn't break them. Things break for their own reasons. The current obstacle system is already a proto-debris field. Authentic version: clusters of obstacles concentrated in a short Z range, forcing the player to weave. Density derived from the run seed's star-density values (reuse the same draw).

**N64 implementation:** existing shape system, just cluster Z positions more tightly per-segment.

---

### Gravitational Field *(planned — Act 1/2)*
An invisible zone that pulls the player laterally. The player must fight the stick to hold their course — full correction overpowers it, but inattention drifts them into debris or walls. The pull direction and strength come from the seed. Visually: a faint distortion shimmer at the field boundary (dim translucent quad ring), the field interior having a subtle ambient color shift (slightly warmer, heavier).

**N64 implementation:** in `update()`, add a lateral force `lateralPos += pull_x * pull_strength` while inside the Z range. No geometry required — purely a force applied to the existing player position.

---

### Pulsar Beam *(planned — Act 1/2)*
A dense neutron star off to one side of the rail fires a rotating beam — a lighthouse sweeping in a predictable rhythm. The beam sweeps a wide arc across the play area. The player must time their dodge: hold left, let it pass, return to center. The star itself is a stationary brilliant white-blue point. The beam is two long narrow quads pivoting around it.

**Visual:** tiny blazing point (bright vertex-colored cube), two long quads rotating on a timer. The rotation speed is slow enough to learn but fast enough that hesitation kills.

**Gameplay:** purely rhythmic — watch the sweep, move with it, don't fight it. A second beat layer: the pulsar fires an irregular radial ring pulse (same expanding ring primitive) that forces the player to dodge in the gap between the ring and the beam. The irregular pulse cannot be learned by rhythm alone.

---

### Magnetar Starquake *(Act 2)*
A pulsar's dangerous cousin. Twin gamma-ray jets lancing from the poles are permanent and must be avoided on approach. The real threat: an unpredictable starquake fires a full radial shockwave ring — same primitive as the pulsar pulse but irregular timing, so it cannot be anticipated. Nearby metallic debris is pulled into a slow inward spiral by the magnetic field.

**Visual:** brilliant blue-white point, two long thin quad jets from poles, expanding ring on starquake.

**N64 implementation:** rotating quad jets (always on), expanding ring (spawned on irregular timer from seed-derived interval), nearby obstacles get a slow radial drift applied in `obstacles_update()`.

---

### Gamma Ray Burst *(Act 2 — rare)*
The most dramatic event in the universe becomes the most dramatic moment in a run. A distant star at the edge of the playfield suddenly blazes white — the only warning. A few seconds later, a shockwave ring rushes toward the player from that direction. No reaction time on a first encounter; it is a "survive or learn" moment. Used sparingly — maximum once per run, seed-determined.

**Visual:** distant star vertex snaps to full white bloom (`rgba = 0xFFFFFFFF`), full-screen white quad overlay at peak intensity fading over 10 frames, then expanding ring.

**Gameplay secondary effect:** player controls become sluggish for 3 seconds (multiply stick input by 0.3) — the EMP equivalent. No rendering cost; purely a code mechanic applied to input scaling.

---

### Dark Nebula / Stellar Nursery *(Act 2)*
A dense molecular cloud — black, opaque, eating the starfield. The player flies into it and the background stars disappear entirely. Obstacles are still present but visible later (closer draw distance inside the cloud). Bright pink-magenta jets erupt laterally from buried protostars, firing "knot" projectiles across the corridor. Acts as a natural difficulty spike without adding new geometry types — just reduced visibility.

**Visual:** ambient light drops to near-black on entry (smooth lerp over 1 second). Background clear color becomes deep brown-black. Star draw skipped inside the volume. Pink-magenta thin quads for jets. Existing obstacle shapes as knots.

**N64 implementation:** zone entry/exit triggers ambient color lerp in `scene_rails_draw()`. Lateral projectiles are existing shapes launched with a fixed transverse velocity added in `obstacles_update()`.

---

### Rogue Planet with Aurora *(Act 2/3)*
The largest obstacle — a cold, dark planet the rail curves around. Nearly invisible itself (deep red-brown vertex colors, almost black). The hazard is its aurora zone: rings of crackling blue-green light extending well past the surface. Threading the gap between the planet and the aurora boundary without clipping it is the challenge. The planet looms close enough to fill most of the screen as the player skirts past — overwhelming by presence, not complexity.

**Visual:** an icosahedron or subdivided shape at large scale, dark vertex colors. Aurora: a ring of translucent quads fanned around the polar axis, vertex-colored black at base to bright blue-green at tip. Color cycles slowly each frame — no texture needed.

**Gameplay:** the rail bends around the planet (camera target shifts laterally), aurora zone is a collision volume. Touching it deals damage. Threading the magnetic pole gap exactly rewards the player with a brief speed boost.

---

### Stargate Transition *(Act boundary / warp mechanic)*
Directly inspired by the "Jupiter and Beyond the Infinite" sequence from *2001: A Space Odyssey*. Douglas Trumbull created the original using slit-scan photography — a camera tracking slowly toward a vertical slit while artwork moved behind it, each frame a long exposure that stretched the art into infinite receding lines. The result: two walls of colored geometric light converging at a vanishing point, palette cycling continuously from electric blue to violet to orange to acid green.

On N64 this is approximated as a tunnel of colored rectangular frames at increasing Z depths, rushing toward the camera:

```c
// 8 rectangular frames at exponentially increasing Z
// Each frame: 4 quads (top/bottom/left/right strip)
// Color indexed from palette, offset by frame index
// Each tick: shift all frames one step closer, wrap farthest back

static const uint32_t stargate_palette[] = {
    0x0033FFFF,  /* electric blue  */
    0x00AAFFFF,  /* cyan-blue      */
    0x4400CCFF,  /* deep violet    */
    0xFF6600FF,  /* hot orange     */
    0xFFDD00FF,  /* acid yellow    */
    0xCC00FFFF,  /* magenta-purple */
    0x00FF88FF,  /* bright green   */
    0xFF0033FF,  /* hot red-pink   */
};
```

Cost: 64 triangles, one color table lookup per frame. Used as a 10-second transition zone between acts — no obstacles, pure spectacle. Signals a boundary: the player has crossed into something different. Also usable as the visual signature of a warp speed boost (player enters briefly, the rail advances faster, they emerge further along).

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

**Why the 2D layer matters:** The 2D backdrop carpet is what makes space feel infinite. Because it responds to camera angle rather than position, it never gets closer — it simply extends forever in every direction. No matter how far the player travels, the background stars are always at the same apparent distance. That's exactly right. The player should never feel like they're running out of sky.

**Stars you actually fly past:** Layer 3 geometry stars provide the counterpoint — real 3D objects with genuine parallax that drift across screen as the player moves laterally. These are what make the player feel like they're actually *moving through* space rather than past a painted backdrop. Both layers are necessary: the 2D carpet for infinity, the 3D stars for velocity.

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

### Starfield — 2D Skybox, Not 3D Space (SF64 Architecture)

Star Fox 64's stars are **entirely 2D**. Stars have no Z coordinate — they are screen-space quads rendered with orthographic projection before any 3D scene. The player cannot fly "past" or "through" them. What looks like depth is pure illusion: stars scroll in response to **camera angle** (pitch/yaw), not camera position. The entire field shifts as if it's infinitely far away and you're rotating toward it.

```c
// SF64: star position = stored 2D offset + camera-angle-derived scroll
gStarfieldX = FloatMod(yaw * factor + scrollOffset, starfieldWidth);
gStarfieldY = FloatMod(pitch * factor + scrollOffset, starfieldHeight);
// bx = starOffset[i].x + gStarfieldX  — pure 2D, no Z
```

SF64 generates 3000 stars from a **fixed seed** (same sky every run) and renders them as 1×1 pixel triangle pairs via ortho projection — essentially free fillrate.

**Implication for us:** Our Layer 1 background stars should use the same approach — 2D screen-space quads, ortho projection, scrolling driven by camera angle or rail progress. This gives us hundreds of stars cheaply. Layers 2 and 3 add the actual 3D depth: billboard sprites with parallax, and true 3D geometry stars you can fly near or past. The SF64 technique is the carpet; our foreground layers are the presence.

**Seed-derived variation SF64 doesn't have:** Because we use a run seed (not a fixed one), each run's sky can look subtly different — same PRNG, different initial seed, completely different star pattern. Density per-segment (the Seed Lab "star density" bars) then controls how many of those background stars show in each zone.

---

## Design Philosophy — Questions This Game Does Not Answer

These are open intentionally. Do not resolve them in code, visuals, audio, or text:

- Is becoming the Colossus death or transcendence?
- Is the Colossus alive or dead — does it even know?
- Are the frozen seekers who didn't reach him failures, or did they arrive differently?
- Does scale make the distinction between life and death meaningless?
- How does the Colossus change across runs, and what does that change mean?

The player should carry these questions out of the game with them. Any implementation decision that accidentally answers one of them should be reconsidered. The game's job is to make the questions feel vast and okay, not to resolve them.
