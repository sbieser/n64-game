# Research — Star Fox 64 Deep Dive

Sources: web research + local `C:\dev\devStarfox64Recomp` (the Starfox64Recomp project — a static PC recompilation of SF64 using N64Recomp + RT64, with patches written against the decompiled C code).

Note: The local repo is **not** the original decompilation source (that lives at github.com/sonicdcer/sf64). It is a PC port layer. However, the patch files reference original game globals (`gPlayer`, `gCurrentLevel`, `gLevelMode`, `PLAYERSTATE_START_360`, `LEVELMODE_ALL_RANGE`) which give useful structural insight.

---

## Design Document

No official design document or pitch deck for Star Fox 64 exists publicly. The 2020 Nintendo Gigaleak released source code and audio assets but no written design documentation.

What exists instead is a body of developer interviews:

**Shigeru Miyamoto:**
- On-rails format was a deliberate constraint. He originally wanted full free-flight but concluded it would disorient players. Rails let players experience 3D sensation without the navigation overhead.
- "The first 30 minutes of a game are the most important" — the producer's job is to make that window exciting.
- Designed to feel "movie-like" — real-time voice acting, dramatic camera events, scripted companion reactions are all intentional cinematic tools.
- Approximately 60% of content carried from original Star Fox, 30% from cancelled Star Fox 2, 10% wholly new.

**Takaya Imamura (Art Director):**
- Described SF64 as "the game of my life."
- Went beyond art direction into plot, music direction, and gameplay mechanics.
- Characterized it as "a character game" — companion drama was central, not cosmetic.
- Development began with programmer Kazuaki Morita prototyping what they internally called "Star Box" while learning 3D programming from scratch.

**Difficulty philosophy:** Inverts traditional shooter difficulty — early stages accessible to average players within days, later stages tuned for developers. Escalation is based on skill comprehension, not pattern memorization.

---

## Stage Duration

A single run visits 7 of 15 total stages. Speedrun any% world record is ~22-23 minutes total. Casual first-playthrough estimate:

| Stage type | Speedrun | Casual estimate |
|---|---|---|
| Long corridor (Meteo, Sector X, Area 6) | ~4 min | 6–8 min |
| Short corridor (Corneria) | ~3 min | 4–5 min |
| Short arena (Katina, Bolse) | ~1:45 | 2–3 min |
| Boss stages | ~1:15 | 2–3 min |

**Implication:** Individual stages are 2–8 minutes each, even for casual players. A run feels complete in under an hour. This matches the session design: one run = one session.

---

## Model Polygon Counts

These are actual in-game mesh counts (not cinematic or title screen models):

| Model | Triangles |
|---|---|
| Arwing (in-game) | ~178 |
| Arwing + Fox cockpit detail | ~419 |
| Fox McCloud character alone | ~461 |
| Corneria city background | ~868–973 on screen total |
| Title screen scene total | ~2,334 |

SF64 runs at 60fps on simpler stages. N64 Fast3D microcode handles ~100,000 polygons/second — at 60fps that is ~1,667 triangles per frame budget. The Arwing at 178 triangles uses about 10% of that budget, leaving the rest for enemies, background, and effects.

**What this means for us:** Our player figure (~12 triangles) is smaller than it could be — the Arwing at 178 is the reference for the central moving object. Our Colossus at 20 triangles is also very lean. We have substantial polygon budget to spare. The disproportionate feeling is probably not polygon count but **world scale** — see next section.

---

## Controls — N64 Corridor Mode

| Input | Action |
|---|---|
| Analog stick | Steer ship / move reticle |
| A (tap) | Fire laser |
| A (hold) | Charge laser; auto-locks reticle onto enemy in range |
| A (release while locked) | Fire homing charged shot |
| B | Deploy Smart Bomb |
| Z (tap) | Tilt ship left |
| R (tap) | Tilt ship right |
| Z (double-tap) | Barrel Roll left — deflects standard projectiles during spin |
| R (double-tap) | Barrel Roll right |
| C-Left | Boost (drains boost/brake gauge) |
| C-Down | Brake (drains boost/brake gauge) |
| C-Up | Cycle camera view |
| C-Right | Receive wingman comms |
| Stick-down + C-Left | Somersault (brief reverse arc) |
| Stick-down + C-Down | U-Turn (All-Range Mode only) |

**Boost/Brake:** Shared gauge displayed upper-right. Both drain it; it regenerates over time. Holding longer increases effect.

**Barrel Roll detail:** Double-tap Z or R within a brief window. Ship spins one full revolution on its roll axis. While spinning, generates a shield that deflects most standard projectiles. No contact damage to enemies. Brief cooldown.

**Implications for us:** We have no shooting mechanic so A, B, and C-Right are available for other actions. The barrel roll (double-tap Z/R) is an interesting pattern — a defensive maneuver gated behind a timing gesture rather than a dedicated button. The boost/brake gauge is a resource management layer we might consider for a stamina or dodge system.

---

## Camera System (from camera.c patch)

Camera uses a standard eye + look-at (at) system. Eye follows the ship at a fixed trailing offset — it does not lead or predict. The ship moves within a 2D rectangle relative to the camera; the rail drives forward motion automatically.

The patch code reveals internal state variables:
- `gPlayer[0].state` — player has a state machine; `PLAYERSTATE_START_360` marks the moment of transition from corridor to all-range
- `gCurrentLevel` — level enum (LEVEL_SECTOR_Y, etc.)
- `gLevelMode` — mode enum: `LEVELMODE_ALL_RANGE` vs corridor
- `gCameraShake` — camera shake is a game-level value, not a per-object property

Camera interpolation thresholds (tuned values from the patch):
- Normal levels: interpolate unless velocity_diff > 200 world units, at_dist > 150, or eye_dist > 300
- During all-range transition: always interpolate
- During camera shake: always interpolate

**Three camera views available:** standard (behind + above), approach (tighter framing), cockpit (first-person, space stages only). Player controls C-Up to cycle them during play.

---

## On World Scale and Proportion

SF64 runs at a 4:3 aspect ratio, 320×240 pixels. At 60fps and with 178-triangle ships, the game can afford more scene geometry than we currently use.

The subjective "disproportionate" feeling in our test project is most likely a **world scale and FOV** issue rather than polygon count. In SF64:
- The Arwing is physically small relative to the play area — you can fly to either edge of the screen and the ship is maybe 15% of screen width
- Obstacles and enemies are typically larger than the Arwing, or similarly sized — contrast creates threat legibility
- The camera distance is tuned so the ship reads clearly but doesn't dominate the frame

In our project:
- FOV is 70° with near clip 10 and far clip 200
- Player figure is 4–5 world units across; play area is 70 wide × 36 tall
- This makes the player figure roughly 4/70 = 5.7% of the lateral play area — may feel too small
- Obstacles at hitRadius 7 and similar visual scale may feel comparable to the player, removing contrast

Calibration options: scale the player figure up, or shrink the play area, or move the camera closer. The goal is that the player figure reads clearly on screen and obstacles feel meaningfully larger or at least distinctly different in shape.

---

## Stage Complexity — Decomp Analysis

Source: `sonicdcer/sf64` repository, cloned locally at `C:\dev\sf64`. Browsed the master branch via local file system. Key files: `include/sf64object.h`, `include/sf64event.h`, `src/engine/fox_edata_info.c`, `src/engine/fox_enmy.c`, `src/overlays/ovl_i5/fox_ma.c`, `assets/yaml/us/rev1/ast_macbeth.yaml`.

---

### How Planet Stage Objects Work

SF64 planet stages populate their world via an **ObjectInit array** stored as binary ROM data, one array per stage. Each entry is a 20-byte struct:

```c
typedef struct {
    f32 zPos1;   // primary Z position
    s16 zPos2;   // secondary Z (used for path/switch positioning)
    s16 xPos;
    s16 yPos;
    Vec3s rot;
    s16 id;      // OBJ_SCENERY_*, OBJ_ACTOR_*, OBJ_ITEM_*
} ObjectInit; // 0x14 bytes
```

All objects for a stage — terrain panels, walls, enemies, items, the boss — live in a single flat array. A sentinel entry (`id == OBJ_INVALID`) marks the end. The engine loads every object in the array at stage start and activates them as the player approaches (via per-object draw distances). There is no dynamic DFS streaming of scenery objects. The total binary data fits in the stage's ROM segment.

Terrain in ground stages (Corneria, Macbeth, Titania) is handled separately: `fox_ground.c` generates a grid of `Vtx` tile geometry that scrolls past the camera. This tile geometry is procedurally updated each frame and is distinct from the placed scenery ObjectInit entries. The two systems coexist: scrolling tile floor + individually placed wall/structure objects.

---

### Object Counts by Stage

Calculated from ROM segment offsets in the YAML asset manifests. Each ObjectInit entry is 20 bytes.

| Stage | Placed objects | Notes |
|-------|---------------|-------|
| Solar (fire planet) | **353** | Fire/lava rails stage, Vulkain boss |
| Aquas (underwater) | **604** | Full submersible 3D corridor, slowest pace |
| Corneria On-Rails | **804** | Tutorial planet, city/carrier setting |
| Corneria All-Range | **530** | Separate array for the mid-stage mode switch |
| Macbeth (train) | **902** | Most complex planet stage |

Macbeth also has three additional smaller arrays:
- `aMaCsLongTrainObjects`: 17 entries (train car formation for the opening cutscene)
- `aMaLongTrainObjects`: 17 entries (alternate long train formation)
- `aMaShortTrainObjects`: unknown size (short train formation)

The 902-entry main array includes: 13 train track segment variants, 8 railroad switch types, buildings, walls, towers, weapons factory, distance markers, terrain bumps, floor panels, and all enemy types.

---

### Stage Length — Macbeth

The train track geometry entries in `fox_ma.c` span world-space Z positions from **+5,174 to −507,035** — the entire pre-placed track. This is the absolute world coordinate range of physical track objects.

The player's traveled path is shorter. The boss fight (Mechbeth) triggers when `gPathProgress = 115,930` and `player->pos.z = −111,130`. These sum to approximately 4,800 units of starting offset, confirming the player travels **~115,930 world units** of path before the boss encounter.

After the boss, the player continues to the stage exit. Total path including boss sequence: probably 130,000–140,000 units.

At SF64's standard on-rails forward speed the stage takes 4–6 minutes to traverse. The pre-placed track extends much further than the player's path because the train locomotive runs ahead of the player — the track must exist at the train's position before the player reaches it.

---

### Draw Distances

Each entry in `gObjectInfo[]` (fox_edata_info.c) includes a `drawDist` field in world units. Objects outside this radius from the player are skipped by the renderer.

Selected Macbeth values:
```
MA_BUILDING_1         300.0f
MA_BUILDING_2         400.0f
MA_TOWER              200.0f
MA_WALL_1            1500.0f
MA_WALL_2            2500.0f
MA_WALL_3/4          2000–2500.0f
MA_FLOOR_1/2/3/4     1100–1900.0f
MA_FLOOR_6           3000.0f
MA_TRAIN_TRACK_3     15000.0f  ← visible from far ahead (the running track)
MA_TRAIN_TRACK_4     15000.0f
MA_TRAIN_STOP_BLOCK  15000.0f
MA_LOCOMOTIVE        20000.0f  ← always visible once spawned
MA_TENDER_CAR        20000.0f
```

The long-draw-distance tracks (15,000–20,000 units) are what keep the approaching train visible across the full final stretch of the stage. Close-detail objects (buildings, small walls) use tight distances (200–400 units) to concentrate polygon budget.

---

### Environment Struct

Each stage defines an `Environment` struct (from `sf64level.h`):

```c
typedef struct Environment {
    s32 type;          // LEVELTYPE_PLANET or LEVELTYPE_SPACE
    s32 unk04;
    u16 bgColor;       // background sky color (packed RGBA)
    u16 seqId;         // music sequence
    s32 fogR, fogG, fogB;
    s32 fogN, fogF;    // fog near/far distances
    Vec3f lightDir;
    s32 lightR, lightG, lightB;
    s32 ambR, ambG, ambB;
} Environment;
```

This is exactly what we configure per-stage in our game: fog color and range, directional light direction and RGB, ambient RGB, background color. One struct per stage defines the entire look.

---

### Implications for Our Game

**Object scale:** Our planet stages are free-roam descent corridors, not long on-rails paths. We don't need 353–902 objects. A realistic target is **60–120 objects** total per planet stage (terrain chunks + placed scenery + vents + landmarks). This is well within N64 RAM — a 100-entry ObjectInit array is 2,000 bytes.

**Terrain strategy:** For our planet stages, the terrain IS the placed scenery — we don't have a separately scrolling ground tile system, because our stages descend rather than fly forward. Each crevasse wall section, cavern ceiling panel, and floor slab is a separate `.t3dm` model placed from an ObjectInit-equivalent array. Draw distance culling handles what's visible.

**Stage length:** We target 5,000–8,000 world units of path per planet stage at our contemplative pace (~7 units/second on environmental rails). That's 12–19 minutes of traversal. At this scale, loading all terrain geometry at stage start (no DFS streaming) is practical — 100 terrain chunks × 40 triangles × ~720 bytes of vertex data = ~72 KB, trivial.

**Draw distances for our stages:** Planet terrain panels: 600–1,000 units. Vents: 300 units. Billboards: match the zone's full visible extent (e.g., gas giant: 3,000 units from anywhere in the surface zone). Pioneer: 300 units (close approach required). Ghost ships: 200 units.

**Space stages:** No terrain geometry at all. Stage geometry is purely: background starfield (2D, not world objects), a few 3D landmark objects (proto-stars, nebula quads, foreground star geometry), and the Pioneer. Total triangle count for a space stage is under 100 tris. All complexity is signal, oxygen, and navigation — not geometry.

---

## All-Range Mode

Several stages break from rails entirely. Identified stages with full all-range: Fichina, Katina, Bolse, Sector Z, hard-path Venom approach. Corneria easy-path mid-stage switch.

The transition to all-range (`PLAYERSTATE_START_360`) is triggered by game events mid-stage, not by player choice. It's used specifically for dogfight scenarios (aerial combat in open space) and the final boss. The feel is fundamentally different — threat comes from all directions, strategy replaces pure reflexes.

**Relevance for The Colossus:** We discussed the possibility of modes beyond pure rails. An all-range-style zone near the Colossus — where forward momentum stops and the player circles him in free space — would follow this established N64 precedent exactly. The camera.c patch shows this transition is handled as a player state change, not a separate scene.
