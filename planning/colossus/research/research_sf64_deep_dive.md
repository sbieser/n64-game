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

## All-Range Mode

Several stages break from rails entirely. Identified stages with full all-range: Fichina, Katina, Bolse, Sector Z, hard-path Venom approach. Corneria easy-path mid-stage switch.

The transition to all-range (`PLAYERSTATE_START_360`) is triggered by game events mid-stage, not by player choice. It's used specifically for dogfight scenarios (aerial combat in open space) and the final boss. The feel is fundamentally different — threat comes from all directions, strategy replaces pure reflexes.

**Relevance for The Colossus:** We discussed the possibility of modes beyond pure rails. An all-range-style zone near the Colossus — where forward momentum stops and the player circles him in free space — would follow this established N64 precedent exactly. The camera.c patch shows this transition is handled as a player state change, not a separate scene.
