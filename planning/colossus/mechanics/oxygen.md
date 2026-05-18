# Oxygen

*The universe kills through indifference. Oxygen is the resource that makes this literal.*

---

## The Resource

The player has a finite oxygen supply. It depletes based on movement state — see `controls.md` for the ship movement model. The short version: rotation is nearly free, thrust costs, boost costs more. Stillness is sanctuary but not infinite safety — the universe is still indifferent.

---

## Replenishment — Ghost Ships

Other seekers who died in the same space left their ships behind. A wrecked ship still has residual oxygen — a supply that ended when they did, still present, still accessible. The player can approach a ghost ship and take what's left.

---

## The Charge System

Each ghost has a **charge count** (target: 5).

- Each visit, the player can take oxygen from a ghost once — that uses one charge.
- Charge counts are saved to SRAM and never reset. They only go down.
- At 0 charges: the ghost is permanently an empty shell. No vent. Dark and static. Never usable again.
- Within a single visit: each ghost can only be used once regardless of remaining charges. This prevents camping a single ghost. The per-visit drain flag is runtime-only — not saved to SRAM.

**Why this works:** Early visits lean on a handful of nearby ghosts with fresh charges. Over many visits, familiar ghosts slowly deplete. When the nearest ghost runs dry, the player is gently pushed to either die deeper (leaving fresh ghosts further in) or push further before dying. The space near a recurring death spot gets used up. The game nudges forward without locking out.

---

## The Loop

- Player dies → becomes a ghost at that position → that ghost has oxygen
- Future visits find that ghost → take its oxygen → get further
- The world accumulates the player's history as a survival resource

Ghost death positions are meaningful: they mark where someone ran out of oxygen mid-thrust — the end of a committed reach. The ghost is at the furthest point of a deliberate attempt.

---

## Ghost Visual States

The ghost's visual state communicates oxygen availability without instruction.

**Undrained (charges remain, not yet used this visit):**
Tumbling wreck + faint atmospheric vent particles drifting from one point on the hull. Cold blue-white, barely visible, irregular. Something is still happening here.

**Used this visit (charges remain, already drained this attempt):**
Tumbling wreck, vent still present (charges remain), but unavailable until next visit. The vent is honest — there is oxygen here, just not for you right now.

**Permanently depleted (0 charges):**
Tumbling wreck only. No vent. Completely static and dark. Gone for good. Its debris field remains as a landmark.

**The vent effect:** A wrecked ship still has pressurized air inside. A hull breach leaks it slowly into vacuum — a faint trickle of tiny cold blue-white particles. Physically grounded, not decorative. There is no smoke in space; nothing burns. What vacuum actually does: tumble, shed debris, vent residual pressure. The vent is the honest version of "oxygen here."

---

## SRAM Persistence

- Ghost positions: saved permanently. Accumulate across all visits. Never removed.
- Ghost charge counts: saved permanently. Decrease with use. Never restored.
- Per-visit drain flag: runtime only. Not saved. Each visit starts with all ghosts usable (subject to remaining charges).

---

## Open Questions

- What is the indicator for the player's own low oxygen — a sound, a color shift, both?
- Does oxygen persist as a mechanic beyond Stage 1, or does it recede as new mechanics take over?
