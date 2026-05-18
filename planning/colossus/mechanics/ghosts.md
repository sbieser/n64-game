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

## The Loop

- Player dies → ghost placed at that position → ghost has oxygen charges
- Future visits find that ghost → can take oxygen → get further
- The player's own death history becomes the resource that makes the journey possible

At some point the player may find a ghost in a position they recognize — somewhere they've been before. An echo of their own path. They were already leaving traces. Future seekers will follow them as they have followed others.

See `mechanics/oxygen.md` for the full oxygen/charge system.
