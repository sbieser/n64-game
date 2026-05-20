# Stage 1 — The Search

*The void. You are here. Something is out there.*

---

## Mode

Free-roaming. No fixed path. No explicit objective. The player has full control of movement in all directions.

---

## Controls

Stage 1 is the first expression of the base controls — full free-roam, no auto-forward, full player thrust authority. See `mechanics/controls.md` for the complete movement and oxygen/thrust mechanics.

The seeking loop — stop, rotate, listen, find direction, thrust — is introduced here and documented in `mechanics/seeking_mechanic.md`.

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

Stage 1 introduces the oxygen mechanic. The full mechanic — charge system, ghost replenishment loop, visual states, SRAM persistence — is in `mechanics/oxygen.md`. The full ghost mechanic is in `mechanics/ghosts.md`.

**What is specific to Stage 1:**

The space starts nearly empty. One ghost, one tank, a long signal to follow. You may not make it on the first visit.

Over visits, ghost ships accumulate where previous attempts ended — clustered near the beacon but not at it, stopped short. Early familiar ghosts slowly deplete their charges. As the nearest ghosts run dry, the player is nudged deeper. The accumulated dead make the journey possible.

The pattern of ghosts across many visits tells the story of the search without words: a field of wrecked ships in the void, all roughly oriented the same direction, all stopped short.

---

## The Intentional Friction

Stage 1 will lose some players. The implicit goal, the absence of instruction, the strangeness of the signal — these will feel like a broken game to some people.

This is not a flaw. The players who stay are the ones who seek. The game is finding its audience by being itself from the first moment. Do not add a tutorial. Do not add a waypoint. Do not explain the signal.

---

## Ghost Narrative in This Stage

Stage 1 ghosts are people who got lost. Ghost mechanics, visual states, and SRAM persistence are in `mechanics/ghosts.md` and `mechanics/oxygen.md`.

**What is specific to Stage 1:**

Player-death ghosts render as wrecked ships — not humanoid figures. The `ghost_reacher` model (the frozen humanoid, still reaching forward) is reserved for the beacon ghost at the signal source. That distinction is the first thing the player learns about what ghost form means.

The beacon ghost is the only one that was *placed* — not the result of a player death. Everyone else stopped short. The beacon ghost got here.

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
