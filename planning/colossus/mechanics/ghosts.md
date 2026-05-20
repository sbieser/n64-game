# Ghosts

*Every seeker who dies becomes a ghost at that position. The world accumulates the history of every attempt. Ghosts are world state — they persist permanently and never move.*

---

## What a Ghost Is

When the player's oxygen runs out, the ship drifts to a stop and becomes a ghost at exactly that position. The ghost is saved to SRAM immediately. On every future visit to that stage, the ghost is there.

Ghost positions are the player's death history made visible and useful. They are not punishment. They are evidence.

---

## Ghost Forms

Not all ghosts look the same. The form communicates what kind of ghost it is.

**Wrecked ship** — player-death ghosts. The ship the player was flying, now dark, tumbling, debris scattered nearby. This is what most ghosts are. Cold coloring, no attitude control, slowly rotating.

**ghost_reacher** (humanoid figure, still reaching forward) — reserved for specific narrative ghosts placed by the game. Not player deaths. The beacon ghost in Stage 1 is a ghost_reacher. These are exceptional — someone who got somewhere, stopped, and is still oriented toward what they were seeking.

The distinction teaches the player something about what they're looking at without explanation.

---

## Ghost Behavior

**Tumble:** Ghost ships rotate slowly at a constant rate. No attitude control — dead ships drift and spin. Axis and speed are derived from the ghost's position in the array so no two spin identically.

**Debris field:** Hull panels, a strut, a small component box scattered nearby. These rotate with the main hull as one slowly tumbling mass.

**Flicker:** Ghost lighting alternates between two ambient values every frame (ghostAmbientA / ghostAmbientB). At 60fps this reads as a shimmer — unstable, barely present. Living ships do not flicker. The contrast between stable and flickering reinforces which one is alive.

**Vent particles:** See `oxygen.md` for vent state details. The vent is the visual indicator of oxygen availability.

---

## Ghost Orientation

A ghost's last heading — the direction it was facing when it died — is preserved. Over many visits, a field of ghosts all roughly facing the same direction is passive navigation information. The dead point where they were going.

This is not a designed UI element. It emerges from physics: every seeker was following the same signal. They all died facing roughly the same way. The player reads it as pattern recognition, not instruction.

---

## SRAM Persistence

- Ghost **positions** are saved permanently. They accumulate. They never move. They never disappear (except charge depletion changes their visual state — they become static shells rather than venting wrecks, but the wreck remains).
- Ghost **charge counts** are saved permanently. See `oxygen.md`.
- Ghost **per-visit drain flags** are runtime only — not saved.

The world remembers everyone who tried.

---

## Ghost Narrative

Ghosts are the game's passive storytelling. Their positions, orientations, and density across a stage tell the history of who came here and what happened to them.

Stage 1: ghosts cluster near the beacon but not at it. Stopped short. The pattern says "many tried, all fell here."

Later stages: ghost density shifts. Clusters form near hard obstacles or strange geometry. The accumulation of deaths is a map of where seeking failed — and therefore a guide for where to be careful.

The player is always reading evidence. Ghosts are part of the evidence.

---

## Ghosts on the Return Journey

On the forward journey, ghost ships are resources and navigation tools — oxygen, orientation, the accumulated testimony of previous seekers.

On the return journey they are witnesses. The player passes them again with different eyes. These were people who got this far and no further. The player has been where they were trying to go. What does it mean to pass them now, heading back?

No interaction changes. The ghost ships are the same ghost ships. Oxygen charges still function if available. What changes is what the player brings to the encounter.

---

## The Loop

- Player dies → ghost placed at that position → ghost has oxygen charges
- Future visits find that ghost → can take oxygen → get further
- The player's own death history becomes the resource that makes the journey possible

At some point the player may find a ghost in a position they recognize — somewhere they've been before. An echo of their own path. They were already leaving traces. Future seekers will follow them as they have followed others.

See `mechanics/oxygen.md` for the full oxygen/charge system.

---

## Ghost Appearance by Environment

In space, ghost ships tumble indefinitely — vacuum preserves them. No force acts on them except their last slow spin. Planets don't work this way. A ghost on a planet is shaped by what that planet does to things left alone.

| Environment | Ghost Appearance | Resource Provided | Notes |
|---|---|---|---|
| Space (void) | Tumbling wreck, debris field, flicker lighting | Oxygen charges (5 max, SRAM-persisted) | Accumulates into fields across many visits |
| Ice moon | Frozen in place, partially encased in ice | Thermal charge (residual reactor heat) | Marks exactly where the cold won |
| Ocean interior | Pinned to seafloor by pressure, or swept to current's edge | [in design] | |
| Storm world | Charred hull, debris scattered by wind, partially buried | [in design] | |

The ghost ship's visual state still communicates resource availability — a faint thermal shimmer from a ship still producing reactor heat, gone dark when depleted. See `mechanics/oxygen.md` for the full charge state logic; the same system applies to thermal charges and whatever the other planetary stages use.

---

## The Planet Doesn't Remember

In space, ghosts accumulate across every visit. The void preserves them forever. Fields of dead seekers all facing roughly the same direction — the accumulated testimony of everyone who tried. The space remembers.

Planets don't. The ocean sweeps wrecks along its current and pins them somewhere anonymous. Ice buries what the cold kills. Storms scatter debris until it's indistinguishable from rock.

On planetary stages there is no accumulated field of testimony. No field of seekers pointing the direction. Only the single ancient ghost_reacher at the stage's end, and — somewhere along the rail — the wreck of your own last attempt if you've died here before.

Your death ghost still appears on planetary stages. That consistency is important: the world always remembers you. What changes is that only your ghosts accumulate here, shaped by what the planet did to them. Not a field of predecessors. Just your own history made visible — showing where the planet won last time.

---

## Ghost as Death Marker on Rails

On rail stages the ghost's position serves a different purpose than in open space. In the void, ghost clusters mark where seeking is hard, the signal is faint, oxygen is scarce. On rails the direction is decided — you're surviving, not navigating.

A ghost along the rail marks precisely where the planet won. The frozen ship on the ice moon, encased exactly where thermal reserves ran out. The charred wreck on the storm world, struck at that exact bend. Not testimony pointing toward the goal — evidence pointing at where to be careful.

The player reads it the same way they read everything else in this game: as evidence. What happened here. Where to pay attention.
