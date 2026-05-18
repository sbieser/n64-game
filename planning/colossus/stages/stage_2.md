# Stage 2 — The Wake

*He has been here. Something enormous passed through this space. You are moving through what it left.*

---

## Mode

Transitional. Still nominally free-roaming, but the space is no longer empty. A lateral drift the player feels but cannot immediately explain. Something is pulling — not strongly yet, not enough to name.

---

## What Is Happening

The player has found the direction. They are moving through the Colossus's wake — the aftermath of his passage. Debris from worlds that used to exist. Evidence of something incomprehensibly large moving through this space before the player arrived.

Stage 2 tells the player about the Colossus before they ever see him. The scale of the destruction implies the scale of the thing that caused it.

---

## The Pull Begins

The drift is subtle at first. The player may correct for it without noticing it. Over time it becomes undeniable — something is exerting force. The player is not yet on rails, but they are no longer entirely free.

The transition from Stage 2 to Stage 3 (full rails) should be imperceptible at the moment it happens. No announcement. No cutscene. The player should realize they've lost forward control only after they've already lost it.

### How the Transition Works (Technical Design)

A single float — `control_authority` — slides from 1.0 (full free-roam) to 0.0 (full rails) over the length of this stage. Player input and gravity fill the complement of each other:

```
player_velocity_z  = player_input_z * control_authority
gravity_velocity_z = rail_speed     * (1.0 - control_authority)
actual_z_movement  = player_velocity_z + gravity_velocity_z
```

At 1.0: the player drives everything. At 0.0: the rail drives everything. At 0.5: a genuine tug of war — the player can resist the pull but cannot stop or reverse. No single frame feels different from the previous one.

The corridor bounds narrow by the same factor — `LATERAL_MAX` and `VERTICAL_MAX` shrink proportionally over the same distance. Wide open space becomes a channel without a wall ever visibly appearing.

The perceptual key: if the player is occupied — dodging debris, finding oxygen from ghosts, reading the wake — they are not testing the edges of their movement. They discover they're on rails only when they try something the rails won't allow. By then, they've been on rails for a while.

**Stage 2 is the transition mechanism.** The whole stage is the gradient. The player enters free and exits captured.

*Not yet implemented. The current test project (scene_rails.c) starts at full rail speed with no free-roam mode. This will need a new scene type that blends the two.*

---

## Oxygen

Ghosts from Stage 1 may appear here — seekers who made it further than most. Oxygen replenishment still possible but sources are sparser. The player is deeper in.

---

## Atmosphere

- The void gives way to something. Geometry begins to appear — dark, irregular, slow-moving debris.
- Color temperature still cold, but the character of the darkness has changed. This is not empty space. This is aftermath.
- Audio: the signal is still present but now accompanied by something else — a low, structural sound, felt more than heard. The Colossus's presence before his appearance.

---

## Visual Identity

Things that were whole and are now slightly apart. Not exploded — undone. The Colossus's passage separated things that used to be together through gravity alone, over time. A planet doesn't explode in his wake; it comes apart slowly, its pieces still roughly in formation, still following the same orbital memory, just no longer bound.

- Large dark masses moving very slowly relative to each other. The player threads between them.
- A directionality to everything — debris oriented the same way, as if pushed by a single vast passage.
- Surfaces that were once interiors, now exposed.
- Faint warm edges on the leading faces of debris — residual heat from tidal compression during his passage. Not fire. Old warmth.

The feeling is aftermath silence, not destruction energy. He didn't rage through here. He moved through, and things came apart in his wake the way water comes apart around a hull.

## Audio Identity

Low structural resonance — vibrations from something that vast take a long time to decay. Felt more than heard. The signal from Stage 1 is still faintly present but distorted, as if passing through something dense.

## Open

- Is there a specific landmark or moment that defines this stage, the way the ghost-at-beacon defines Stage 1?
- Does the debris density peak somewhere mid-stage — a denser region the player must navigate — then thin toward the rail transition?
- How does the narrowing corridor manifest visually before the player hits its edges?
