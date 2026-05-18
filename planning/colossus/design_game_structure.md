# Game Structure

*The structural and architectural design of the game. For themes, the nature of the Colossus, and emotional register — see `colossus.md`.*

---

## Structure

The game is stage-based, not run-based. Completing a stage unlocks the next one. Stages can be revisited freely. The player progresses over as many sessions as they need.

**What persists:** Stage unlock state. Ghost positions and charge counts in every stage.  
**What does not persist:** The player's position within a stage. Each visit starts fresh from the beginning.

Ghosts accumulate permanently — they are world state, not session state. Stage 1 on a first visit is nearly empty. Stage 1 after many visits is populated with the history of every attempt. The space remembers everyone who tried.

**Save backend:** SRAM (32 KB cartridge SRAM). Compatible with SummerCart 64 and EverDrive flash cartridges, and all major emulators. Stage unlock state fits in a single byte bitmask. Ghost pools for all stages fit comfortably within the 32 KB limit.

---

## The Arc

```
Many seeking stages → Threshold crossing → Rail stages (last few) → Arrival
Free-roam seeking  → Physics takes over → Being carried          → Inside
```

The seeking stages vary in number, length, and challenge. The signal develops. The space grows stranger. New mechanics and complications to seeking are introduced gradually — sprinkled across stages as the spaces demand, not unlocked in a fixed sequence.

The threshold crossing isn't a moment — it's a gradual realization. The player notices they're drifting. Then they notice they can't stop it. Then they understand they've been inside the gravity well for a while now.

The transition to rails is physics, not a game mechanic. The Colossus is not pulling you. It is simply massive. You got close enough.

---

## Stages

Detailed design for each stage lives in `stages/`. Summaries here.

**Stage 1 — The Search**  
The void. A signal with no explanation. The first ghost at the signal source. The oxygen mechanic is introduced. See `stages/stage_1.md`.

**Stage 2 — (in design)**  
The space the Colossus passed through. Evidence of creation — new stars forming in its wake. The drift begins but the player still has thrust authority. Seeking in a noisier, more present environment. See `stages/stage_2.md`.

**Stage N — Rails Begin (stub)**  
Some stage deep in the sequence where the threshold has been crossed and the player can no longer thrust free. They can steer but not stop. The Colossus is ahead, not seen. Stage number not decided.

**Stage N+1 — Arrival (stub)**  
He is here. No combat. No objective. The camera spirals in. The hold. Six seconds inside something vast. Then the selector. No explanation. No score. The last stage, whatever number it carries.

---

## Open Questions

- **How many seeking stages?** Many — the seeking mechanic has room to develop across a large number of stages before rails begin. Exact count not decided.

- **Oxygen scope:** Persists through all stages until arrival. At the arrival stage the vessel is shed — the player no longer needs oxygen because they are no longer in a ship. The oxygen mechanic ends not through depletion but through transformation. See `mechanics/oxygen.md`.

- **Stage length:** Targeting 10–20 minutes per stage for a first visit. Return visits shorter — the player knows the space. Complete first playthrough roughly 45–90 minutes total.

- **What does completing a stage feel like?** Arrival, not victory. No fanfare, no score. Something changes — the next stage opens — and that is all.

- **How explicit is the ghost narrative?** The player finds a frozen figure at the beacon. They are not told it is a previous seeker. They can infer — or not. This is consistent with the game's philosophy of holding questions open.
