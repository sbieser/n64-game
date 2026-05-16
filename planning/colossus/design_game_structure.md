# Game Structure — Design Notes

*Captured from design session. Ideas in progress — nothing here is final.*

---

## The Arc

The game's structure mirrors its emotional arc. The player begins with full agency and ends with none — and that loss of control *is* the transformation.

```
Stage 1      Stage 2       Stage 3        Stage 4       Arrival
Free-roam → Drift begins → Rails / pull → Full rails → All-range / orbit
(seeking)   (something     (he has you,   (the universe  (the merging)
             has you)       you dodge)      resists)
```

The transition from free-roam to rails is not a game mechanic choice — it is the Colossus's gravity taking over. You were seeking. Then you were found.

---

## Stage 1 — The Search

**Mode:** Free-roaming in the void. No fixed path. No explicit objective.

**What the player is doing:** Following a signal. The signal is not explained. The game does not tell the player what it is or where it leads. They hear something — irregular, strange, almost organic — and it gets louder and more centered as they approach. The player must decide to follow it.

**This is intentional friction.** Stage 1 will lose some players. The ones who stay are the ones who seek. That self-selection is not a flaw — it is the game finding its audience.

**Navigation:** 3D positional audio. The signal pulses. Pan and volume shift as the player turns and moves. No waypoint, no map, no arrow.

**Resource:** Oxygen. The universe kills through indifference, not hostility. A supply that depletes. The player must manage how far they search against how long they have. Open questions:
- Does oxygen deplete faster when boosting?
- Where does the player replenish? Drifting salvage from other seekers? Residual atmosphere on dead debris?
- The answer shapes what the space feels like

**The beacon is a ghost.** The signal the player is following was left by someone who came before — another seeker who got further than most, close enough to leave a mark, and didn't make it back. Their ghost is near the beacon. The player arrives at a frozen figure in the void, still reaching forward. The signal originated from *them*.

This means stage 1 ghosts tell a different story than later ghosts:
- Stage 1 ghosts: people who got lost searching
- Stage 3–4 ghosts: people who almost reached him

**When oxygen runs out:** The player drifts to a stop and becomes a ghost at exactly that position. Recorded to EEPROM. On the next run, that ghost is there — visible proof of how far they made it. Stage 1 deaths cluster near the beacon but not at it. The pattern of ghosts tells the story of the search.

---

## Stage 2 — The Pull Begins

**Mode:** Still nominally free-roaming, but something is different. A lateral drift the player feels but can't immediately explain. Early debris — the dead worlds the Colossus has already passed through. Obstacles, but sparse.

The player still has control. But they are no longer in empty space.

---

## Stage 3 — On Rails

**Mode:** Full rails. Forward motion is no longer the player's. The Colossus's gravity has them.

All phenomena active: gravitational fields, harder obstacles, stranger geometry. The environment grows sparser and more alien as the signal — now clearly directional, clearly *vast* — dominates everything.

The player can only steer left/right/up/down within the corridor. They cannot stop. They cannot go back.

---

## Stage 4 — The Approach

**Mode:** Rails at full intensity. Obstacles thin out toward the end — not because the path gets easier, but because nothing else matters at this scale. The universe stops resisting. He's just ahead.

Color temperature shift complete: deep cold blue → warm cosmic orange.

---

## Arrival — The Colossus

**Mode:** All-range. Forward motion stops. He is here.

No combat. No objective. The player orbits or drifts toward him. The camera spirals in. At some distance the vessel falls away and the figure continues alone. The geometry of the Colossus surrounds the camera from inside.

The hold. Six seconds of being inside something vast.

Then: the selector. No explanation. No score. No ending screen.

---

## Open Questions

- **How many stages?** SF64 runs 7 stages per run. Our arc fits naturally into 4–5. More stages could add texture between the defined beats — an additional strange zone in Stage 2, or two distinct phases of the rail. Not decided.

- **Does oxygen persist beyond Stage 1?** It could be Stage 1 only — a mechanic that teaches you the universe is indifferent and then recedes, letting new layers take over. Or it could persist at reduced importance. The latter risks diluting the later stages' distinct identity.

- **How explicit is the ghost narrative?** The player finds a frozen figure at the beacon. They are not told it is a previous seeker. They are not told the signal came from them. They can infer — or not. This is consistent with the game's philosophy of holding questions open.

- **Stage count and run length:** Targeting a 30–50 minute casual run (SF64 reference: ~45 min). Each stage probably 5–10 minutes. Means 4–6 stages is the right range.

- **What does Stage 2 feel like as a place?** The void gives way to something. A debris field? A dying star system? What is the visual and audio identity that makes it feel distinct from Stage 1 and Stage 3?
