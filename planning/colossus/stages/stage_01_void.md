# Stage 1 — The Search

*The void. You are here. Something is out there.*

---

## Setting

The void. Stars in every direction, no landmarks, no context. The player is alone in the largest possible silence.

Something is out there. The game does not say what, or where, or how far. There is a sound — irregular, not mechanical, almost organic — that shifts in panning and volume as the player moves. Louder means closer. More centered means facing it. The player must decide to trust their ears.

At the end of the signal: a frozen figure, seen through the windshield as you close in. Another seeker who came before, stopped here, still reaching forward. The signal was coming from them.

---

## Mode

Free-roaming. No fixed path. No explicit objective. Full player thrust authority in all directions.

---

## Threat

Oxygen. The full mechanic — depletion rates, ghost charge system, visual states, SRAM persistence — is in `mechanics/oxygen.md`. The full ghost mechanic is in `mechanics/ghosts.md`.

**What is specific to Stage 1:**

The space starts nearly empty. One ghost, one tank, a long signal to follow. You may not make it on the first visit.

Over visits, ghost ships accumulate where previous attempts ended — clustered near the beacon but not at it, stopped short. Early familiar ghosts slowly deplete their charges. As the nearest ghosts run dry, the player is nudged deeper. The accumulated dead make the journey possible.

The pattern of ghosts across many visits tells the story of the search without words: a field of wrecked ships in the void, all roughly oriented the same direction, all stopped short.

---

## Seeking

Audio — volume and pan. The baseline. The seeking loop is introduced here: stop, rotate, listen, find direction, thrust. See `mechanics/seeking_mechanic.md` for the full loop documentation and the signal's sensory arc across stages.

**The signal's ambiguity:**

The nature of the signal is not answered and must never be answered:
- Was it intentional? Did they choose to broadcast, hoping someone would follow?
- Was it automatic — a suit beacon that activated on death?
- Is it a warning? A distress call? An invitation?
- Is the signal the person, somehow, still radiating after death?

Reference: the LV-426 transmission in *Alien* — readable as warning, distress, territorial marker, or something with no human analogue. The ambiguity is the meaning. Hold all interpretations simultaneously.

The player will also realize, eventually, that they are leaving traces too. Flying through space. Dying in space. Whatever the ghost is broadcasting, the player will become.

---

## The Pioneer

At the signal source — the frozen humanoid figure, seen through the windshield as the player draws close. Stage 1's Pioneer is recent enough that its form is clear, the reaching gesture readable, the intention unmistakable. Someone got here. They stopped here. They are still facing what they were trying to reach. The player sees this from inside their own ship: instrument lights below, the Pioneer filling the void ahead.

The beacon ghost is the only placed ghost — not the result of a player death. Everyone else in this stage stopped short. The beacon ghost got here.

Finding it resolves a question and opens a larger one. What was it trying to reach? What is the signal pointing toward beyond this?

---

## Ghost Behavior

Player-death ghosts render as wrecked ships — not humanoid figures. The Pioneer model is reserved for the beacon ghost. That distinction is the first thing the player learns about what ghost form means, without being told.

See `mechanics/ghosts.md` for the full ghost system and `mechanics/oxygen.md` for the charge system.

---

## Return: Memory Fragment

Stage 1 on the return is not a new fragment — it is the accumulation of all fragments recovered across every return stage, resolving into a heading.

The player has crossed every stage again. They carry the river, the chickens, the two houses lit at night, the rain on the drain. In the same void where the journey began, everything assembles. A specific direction. A smaller front house on a grandparents' property. A heading, not a destination.

The credits roll on orientation, not arrival. See `mechanics/memories.md`.

---

## Atmosphere

**Visual:**

The void through the windshield. Instruments below — all eight oxygen squares lit cyan, full tank. The signal screen hunting, no strong direction yet. No arrow, no prompt. Then they hear it.

Not spectacle — presence.

*Layer 1 — Background star carpet:* A dense orientation-responsive starfield — 2D screen-space stars that scroll based on camera angle, never moving closer regardless of player position. The SF64 approach. Color-varied: mostly cold white, occasional blue-white, a few faint warm points. Dense enough to feel overwhelming.

*Layer 2 — Nebula color washes:* Two or three enormous translucent quads at extreme distance, giving different regions of the void distinct color identity. A cold violet region toward the signal. A deep blue region behind. Not objects — a quality of the darkness. Nearly free to render.

*Layer 3 — Foreground 3D stars:* Three or four actual geometry stars close enough to show real parallax as the player moves. That drift is what makes Stage 1 feel like flight rather than floating. Spatial landmarks.

*Layer 4 — Cosmic dust particles:* Twenty to thirty slow-drifting points streaming past the camera during flight, settling when stopped. Pure velocity feedback — the player feels movement through featureless space.

*Layer 5 — The beacon:* A very slow expanding ring pulse, nearly invisible, centered on the Pioneer. Dim, irregular timing — not a clean game-mechanic pulse but something stranger. The moment of noticing — *what is that* — is the discovery. It should not read as "follow me." It should read as "something is there."

**Audio:**

The signal is the dominant sound. Everything else is silence or near-silence. No music, or music that is barely there — texture, not melody. The signal is the music.

---

## Design Note — The Intentional Friction

Stage 1 will lose some players. The implicit goal, the absence of instruction, the strangeness of the signal — these will feel like a broken game to some people.

This is not a flaw. The players who stay are the ones who seek. The game is finding its audience by being itself from the first moment. Do not add a tutorial. Do not add a waypoint. Do not explain the signal.

---

## Open

- What triggers the stage transition — proximity to the Pioneer, interacting with it, or something else? The transition should feel like a consequence of arrival, not a menu.
