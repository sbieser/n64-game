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

The game has two journeys. The Colossus is the hinge — the still center — between them.

```
FORWARD JOURNEY
Many seeking stages → Threshold crossing → Rail stages → Arrival
Exterior signal    → Physics takes over → Carried     → The still center

                        THE COLOSSUS
                     (the hinge point)

RETURN JOURNEY
Reverse rails    → Reverse seeking stages → The void
Agency returns   → Interior signal        → Back to origin
                   (memories)
```

**Forward journey:** The seeking stages vary in number, length, and challenge. The signal develops. The space grows stranger. New sensing modalities are introduced gradually. The threshold crossing is a gradual realization — the player notices the drift, then notices they can't stop it, then understands they've been inside the gravity well for a while. The transition to rails is physics, not a mechanic. The Colossus is not pulling you. It is simply massive.

**The hinge:** Arrival at the Colossus. The hold. The still center. See `stages/stage_4_arrival.md`.

**Return journey:** The player travels back through all stages in reverse. The seeking loop is mechanically identical but the signal inverts — from external cosmic signal to interior memories. Control authority climbs back toward 1.0 as the player moves away from the Colossus. Ghost ships become witnesses rather than resources. The journey ends in the void of Stage 1, visually identical to the opening, the player carrying everything the journey gave them. See `return_journey.md`.

---

## Stages

Detailed design for each stage lives in `stages/`. The rough map below shows the current planned arc — stage numbers and details will shift as design develops.

### Stage Map

Two kinds of rail stages exist and they are different things:
- **Environmental rails** (planetary stages): The terrain channels the player. Gravity, current, storm corridor. Direction is defined by landscape. The player steers within it.
- **Colossus-gravity rails** (late journey): The Colossus's mass has taken hold. The player is being pulled regardless of what they do. They can steer but not stop. This is physics, not terrain.

| Stage | Location | Mode | Primary Mechanic | Ghost_Reacher Age |
|---|---|---|---|---|
| 1 | The Void | Free-roam | Audio seeking (volume + pan) | Recent |
| 2 | Young Star Field | Free-roam | Noise interference | Older |
| 3 | Ice Moon | Environmental rails | Thermal seeking for vents | Old — frozen in ice |
| 4 | Ocean Interior | Environmental rails | Pressure management | Very old — seafloor |
| 5 | Storm World | Environmental rails | Timing, electrical threat | Ancient |
| 6 | Ringed Gas Giant | Free-roam | Visual seeking, gravitational lensing | Ancient |
| 7 | Pulsar | Free-roam | Rhythmic timing | Very ancient |
| 8 | Supernova Remnant | Free-roam | Synthesis — all senses | Ancient beyond reckoning |
| 9 | Dark Nebula | Free-roam | Haptic dominant | Pre-human by millions of years |
| N | Colossus Gravity Threshold | Colossus-gravity rails | Survival, obstacle avoidance | — |
| N+1 | Arrival | Still center | — | — |

Stage numbers are not fixed. The table shows order and character, not final count. Additional stages can be inserted anywhere — more seeking stages in space, more planetary stops, a longer approach to the gravity threshold.

### Stage Notes

**Stage 1 — The Search**  
The void. A signal with no explanation. The first ghost at the signal source. Audio seeking introduced. See `stages/stage_1.md`.

**Stage 2 — Young Star Field**  
The space the Colossus passed through. New stars forming in its wake. Noise interference: stellar EM vs. the organic Colossus signal. Still free-roaming, full thrust authority. See `stages/stage_2.md`.

**Stage 3 — Ice Moon**  
A moon in the Colossus's wake. Environmental rails through ice crevasses and caverns. Thermal seeking replaces audio as primary tool — find geothermal vents or freeze. Ghost_reacher encased in ancient ice. See `stages/stage_3.md`.

**Stages 4–9 — (in design)**  
Ocean interior, storm world, ringed gas giant, pulsar, supernova remnant, dark nebula. Each introduces or develops a mechanic; planetary stages use environmental rails with stage-specific threats; space stages return to free-roam seeking at increasing scale.

**Stage N — Colossus Gravity Threshold (stub)**  
The point where the player realizes they can no longer thrust free. The Colossus's gravity has them. They can steer but not stop. Not a designed event — the player notices when it's already true. Stage number not decided.

**Stage N+1 — Arrival (stub)**  
He is here. No combat. No objective. The camera spirals in. The hold. The still center. Then the selector. See `stages/stage_4_arrival.md` (currently misnamed; will be renumbered when stage count is decided).

---

## Open Questions

- **How many seeking stages?** Many — the seeking mechanic has room to develop across a large number of stages before rails begin. Exact count not decided.

- **Oxygen scope:** Persists through all stages until arrival. At the arrival stage the vessel is shed — the player no longer needs oxygen because they are no longer in a ship. The oxygen mechanic ends not through depletion but through transformation. See `mechanics/oxygen.md`.

- **Stage length:** Targeting 10–20 minutes per stage for a first visit. Return visits shorter — the player knows the space. Complete first playthrough roughly 45–90 minutes total.

- **What does completing a stage feel like?** Arrival, not victory. No fanfare, no score. Something changes — the next stage opens — and that is all.

- **How explicit is the ghost narrative?** The player finds a frozen figure at the beacon. They are not told it is a previous seeker. They can infer — or not. This is consistent with the game's philosophy of holding questions open.
