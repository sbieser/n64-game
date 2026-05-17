# Stage 1 — The Search

*The void. You are here. Something is out there.*

---

## Mode

Free-roaming. No fixed path. No explicit objective. The player has full control of movement in all directions.

---

## Controls

Auto-forward thrust. The ship is always moving in the direction it faces at a slow base speed — the player is never stationary unless dead. The player steers by changing direction, not by choosing when to move.

- **Analog stick:** yaw (left/right) and pitch (up/down) — turns the ship, which changes heading
- **A button:** boost — increases speed temporarily
- The ship gradually aligns to the input direction; movement follows facing

**Traversal is fully 3D.** The signal can be in any direction — above, below, beside. The player pitches toward it and auto-forward carries them there. No explicit vertical thrust buttons needed; heading IS movement. No roll — the world-up vector is locked so the camera always knows which way is up regardless of ship attitude. This prevents disorientation and keeps audio navigation readable.

**Open:** Does oxygen deplete faster when boosting? Not yet decided.

---

## What Is Happening

The player is following a signal. The game does not name it, explain it, or point to it. There is no waypoint. There is a sound — irregular, not mechanical, almost organic — that shifts in panning and volume as the player moves. Louder means closer. More centered means facing it. The player must decide to trust their ears.

At the end of the signal: a frozen figure in the void. A ghost. Another seeker who came before, stopped here, still reaching forward.

The signal was coming from them.

---

## The Signal

The nature of the signal is not answered and must never be answered:

- Was it intentional? Did they choose to broadcast, hoping someone would follow?
- Was it automatic — a suit beacon that activated on death?
- Is it a warning? A distress call? An invitation?
- Is the signal the person, somehow, still radiating after death?

Reference: the LV-426 transmission in *Alien* — readable as warning, distress, territorial marker, or something with no human analogue. The ambiguity is the meaning. Hold all interpretations simultaneously.

The player will also realize, eventually, that they are leaving traces too. Flying through space. Dying in space. Whatever the ghost is broadcasting, the player will become.

---

## Oxygen

The universe kills through indifference. The player has a finite oxygen supply that depletes continuously.

**Replenishment:** Other ghosts. Each ghost in the world has residual oxygen — a supply that ended when they did, still present, still accessible. The player can approach a ghost and take what's left.

**The loop:**
- The player dies → becomes a ghost at that position → that ghost has oxygen
- Future visits find that ghost → take its oxygen → get further
- The world accumulates the player's death history as a survival resource

Early visits: the space is almost empty. One tank. The signal is far. You may not make it.  
Over time: ghost clusters form where previous attempts ended. The accumulated dead make the journey possible.

**Open questions:**
- Does oxygen deplete faster when boosting?
- What is the visual indicator for oxygen level — a sound, a color shift, both?

**Decided:**
- Each ghost has a **charge count** (target: 5). Each attempt, the player can take oxygen from a ghost once — that uses one charge. The charge count is saved to SRAM and never resets.
- At 0 charges: the ghost is permanently an empty shell. No vent. Dark and static. Never usable again.
- Within a single attempt: each ghost can only be used once regardless of remaining charges. This prevents camping a single ghost. The per-attempt drain flag is runtime-only and not saved.
- Ghost **positions and charge counts** are saved to SRAM. Ghost positions accumulate permanently. Charges only go down, never up.

**Why this works:** Early attempts lean on a handful of nearby ghosts with fresh charges. Over many attempts, those familiar ghosts slowly deplete. When the nearest ghost runs dry the player is gently pushed to either die deeper (leaving fresh ghosts further in) or push further before dying. The space near a recurring death spot gets used up. The game nudges forward without locking out.
- The oxygen indicator is a **venting atmosphere effect** — not a UI element, not a flicker. A wrecked ship still has pressurized air inside; a hull breach leaks it slowly into vacuum. This appears as a faint trickle of tiny cold blue-white particles drifting from one spot on the hull. When drained: the vent stops, the ship is completely static. The presence or absence of the vent tells the player everything without instruction.
- Ghost ships **slowly tumble** — no attitude control, dead in space. Each ghost rotates at a low constant rate (axis and speed derived from its position in the ghost array, so no two spin identically). Living ships hold attitude. Dead ones don't.
- The debris field is part of the ghost ship model — hull panels, a strut, a small component box scattered nearby. These rotate with the main hull, as if the whole wreckage is one slowly tumbling mass.

---

## The Intentional Friction

Stage 1 will lose some players. The implicit goal, the absence of instruction, the strangeness of the signal — these will feel like a broken game to some people.

This is not a flaw. The players who stay are the ones who seek. The game is finding its audience by being itself from the first moment. Do not add a tutorial. Do not add a waypoint. Do not explain the signal.

---

## Ghost Narrative in This Stage

Stage 1 ghosts are people who got lost. They cluster near the beacon but not at it — they ran out before arriving, or arrived and couldn't continue.

Their pattern across visits tells the story of the search without words: a field of wrecked ships in the void, all roughly oriented the same direction, all stopped short.

**Ghost visuals:** Player-death ghosts in Stage 1 render as a wrecked ship — hull, broken wing, loose debris field. Dark, cold coloring throughout. Slowly tumbling. The humanoid `ghost_reacher` figure is reserved for the beacon ghost specifically — the one frozen figure at the signal source, still reaching forward.

**Undrained ghost:** Tumbling wreck + faint atmospheric vent particles drifting from one point on the hull. Cold blue-white, barely visible, irregular. The vent says "something is still happening here" without explaining what.

**Partially drained ghost (charges remaining, already used this attempt):** Tumbling wreck, vent still present (charges remain), but player cannot take oxygen again until next attempt.

**Permanently depleted ghost (0 charges):** Tumbling wreck only. No vent. Completely static and dark. The player reads it immediately — that one is gone for good. Its debris field remains as a landmark and part of the accumulating graveyard.

**Why no smoke:** There is no atmosphere in space. Nothing burns. What a wrecked ship actually does in vacuum: tumble, shed debris, vent residual pressure through hull breaches, hold residual heat on exposed surfaces. The vent effect is physically grounded, not decorative.

---

## Visual Design

### First Impression

The player appears in the void. Stars in every direction. Two faint color regions shape the darkness without explaining themselves. A few bright stars sit in true 3D space. Cosmic dust drifts. There is no UI, no arrow, no prompt. Then they hear it. They start moving. A foreground star drifts past. The dust streams. And somewhere out there, something barely pulses.

Not spectacle — presence.

### Layer 1 — Background Star Carpet

A dense field of stars surrounding the player in all directions. Unlike the rail stages where stars are world-space quads distributed along Z, Stage 1 needs a starfield that responds to camera *orientation* rather than position — stars that always feel infinitely far regardless of where the player points. The SF64 approach: 2D screen-space stars scrolling based on camera angle. The player cannot fly toward or past them. They are always at the same apparent distance. This is what makes space feel infinite.

Color-varied: mostly cold white, occasional blue-white, a few faint warm points. Dense enough to feel overwhelming in the good way — you are inside something vast.

### Layer 2 — Nebula Color Washes

Two or three enormous translucent quads at extreme distance, giving different regions of the void distinct color identity. A cold violet region toward the signal. A deep blue region behind. Almost invisible individually — they don't read as objects, only as a quality of the darkness. Together they make the void feel *shaped* rather than uniform, and they give the player subtle directional cues without being explicit.

Nearly free to render. A handful of large quads with alpha blending.

### Layer 3 — Foreground 3D Stars

Three or four actual 3D geometry stars — bright point quads or small crosses — close enough that the player can fly near or past them. When the player moves, these drift across screen with real parallax. That drift is what makes Stage 1 feel like flight rather than floating. A hundred background stars do less work than three foreground ones that visibly move.

These are also spatial landmarks. The player may navigate partly by them.

### Layer 4 — Cosmic Dust Particles

Twenty to thirty small slow-drifting points moving past the player as they fly. When moving they stream past the camera. When stopped they settle. Pure velocity feedback — the player feels their own movement through the dust even in featureless space. Almost no render cost.

### Layer 5 — The Beacon

The most important single visual in Stage 1. The ghost at the signal source needs a visual counterpart to the audio — something faint and irregular that the player may notice before they understand what it is.

A very slow expanding ring pulse, nearly invisible, centered on the ghost. Dim, irregular timing — not a clean game-mechanic pulse but something stranger. The first time the player sees it they may not recognize what they're looking at. The moment of noticing — *what is that* — is the discovery. It should not read as "follow me." It should read as "something is there."

The ghost itself, when found, renders with the existing flicker lighting — alternating ambient every other frame. In the void with almost no other light, this flicker will be eerie and visible from a distance.

### Existing Demo Pieces That Carry Forward

| System | Status | Notes |
|---|---|---|
| Ghost rendering + flicker | Ready | Core to Stage 1's visual language |
| Shapes system | Ready | Sparse debris if needed |
| Color temperature (cold palette) | Ready | Stage 1 stays at the cold end throughout |
| Expanding ring primitive | Ready | Basis for the beacon pulse |

### New Visual Work Required

| Element | Notes |
|---|---|
| Orientation-based starfield | Replace rail starfield with camera-angle-driven 2D carpet |
| Nebula color wash quads | Large translucent background quads, 2–3 regions |
| Foreground 3D stars | 3–4 bright geometry stars with real parallax |
| Cosmic dust particles | 20–30 slow-drifting points, velocity-relative |
| Beacon pulse | Irregular slow ring pulse centered on ghost, very dim |

## Atmosphere

- Deep black, almost no ambient light
- Stars in every direction — the seeker's domain, not the Colossus's yet
- Two faint color regions shape the void without naming themselves
- Audio: the signal is the dominant sound. Everything else is silence or near-silence.
- No music, or music that is barely there — texture, not melody. The signal IS the music.

---

## End Condition

The player reaches the ghost at the signal source. Something changes. The next stage begins.

What triggers the transition is not decided — whether it is proximity to the ghost, interacting with it, or something else. The transition should feel like a consequence of arrival, not a menu.
