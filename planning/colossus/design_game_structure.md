# Game Structure

*The structural and architectural design of the game. For themes, the nature of the Colossus, and emotional register — see `colossus.md`.*

---

## Structure

The game is stage-based, not run-based. Completing a stage unlocks the next one. Stages can be revisited freely. The player progresses over as many sessions as they need.

**What persists:** Stage unlock state. Ghost positions and charge counts in every stage.  
**What does not persist:** The player's position within a stage. Each visit starts fresh from the beginning.

Ghosts accumulate permanently — they are world state, not session state. The void on a first visit is nearly empty. After many visits it is populated with the history of every attempt. The space remembers everyone who tried.

**Save backend:** SRAM (32 KB cartridge SRAM). Compatible with SummerCart 64 and EverDrive flash cartridges, and all major emulators. Stage unlock state fits in a two-byte bitmask (16 bits covers up to 16 stages). Ghost pools for all stages fit comfortably within the 32 KB limit.

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

**The hinge:** Arrival at the Colossus. The hold. The still center. See `stages/stage_arrival.md`.

**Return journey:** The player travels back through all stages in reverse. The seeking loop is mechanically identical but the signal inverts — from external cosmic signal to interior memories. Control authority climbs back toward 1.0 as the player moves away from the Colossus. Ghost ships become witnesses rather than resources. The journey ends in the void of Stage 1, visually identical to the opening, the player carrying everything the journey gave them. See `return_journey.md`.

---

## Stages

Detailed design for each stage lives in `stages/`. The rough map below shows the current planned arc — stage numbers and details will shift as design develops.

### Stage Map

Two kinds of rail stages exist and they are different things:
- **Environmental rails** (planetary stages): The terrain channels the player. Gravity, current, storm corridor. Direction is defined by landscape. The player steers within it.
- **Colossus-gravity rails** (late journey): The Colossus's mass has taken hold. The player is being pulled regardless of what they do. They can steer but not stop. This is physics, not terrain.

Scale progresses from intimate (the void) to cosmic (the dark nebula). Planet stages cluster in the middle — a passage through the Colossus's wake made material. The space stages that follow grow to scales where the journey's origin and destination are both incomprehensible.

| Stage | Location | Mode | Primary Mechanic | Ghost_Reacher | Return: Memory Triggered |
|---|---|---|---|---|---|
| 1 | The Void | Free-roam | Audio seeking (volume + pan) | Recent | Silence → the specific quality of solitude |
| 2 | Young Star Field | Free-roam | Noise interference | Older | Warm lights in cold dark → windows at night |
| 3 | Ice Moon | Env. rails | Thermal seeking for vents | Old — frozen in ice | Cold → a specific winter, a specific room |
| 4 | Volcanic Moon | Env. rails | Dangerous heat — seek cool margins | Very old — sulfur-encased | Heat → something that burned with urgency |
| 5 | Geyser Moon | Env. rails | Direction briefly inverted — plume carries you up | Ancient — at geyser rim, facing outward | Brief wrong direction → a small getting-lost and finding back |
| 6 | Methane World | Env. rails | Fog navigation — brief windows of clarity | Ancient — submerged in methane lake | Almost-familiar wrongness → something seen in the wrong light |
| 7 | Ocean Interior | Env. rails | Pressure management — ghost gives depth | Very old — at Ice VII floor, facing up | Suspension in water → swimming somewhere ordinary |
| 8 | Storm World | Env. rails | Timing — electrical discharge gaps | Ancient — in the eye of a permanent vortex | Turbulence → shelter from it, sound of storm from inside |
| 9 | The Rogue | Free-roam | Gradient thermal — no vent points | Billions of years old — looks new | Total darkness → solitude that felt sufficient |
| 10 | Ringed Gas Giant | Free-roam | Visual seeking — gravitational lensing | Ancient | — |
| 11 | Pulsar | Free-roam | Rhythmic timing | Very ancient | Rhythm → a heartbeat, something that pulsed regularly |
| 12 | Supernova Remnant | Free-roam | Synthesis — all senses simultaneously | Ancient beyond reckoning | Aftermath → something ended, the silence after |
| 13 | Dark Nebula | Free-roam | Haptic dominant — rumble pak | Pre-human by millions of years | Vast dark → the dark that felt like a room |
| N | Colossus Gravity Threshold | Colossus-gravity rails | Survival within the pull | — | — |
| N+1 | Arrival | Still center | — | — | — |

Stage numbers are not fixed. The table shows order and character, not final count. Additional stages can be inserted anywhere — more seeking stages in space, more planetary stops, a longer approach to the gravity threshold.

### Stage Notes

**Stage 1 — The Search**  
The void. A signal with no explanation. The first ghost at the signal source. Audio seeking introduced. See `stages/stage_01_void.md`.

**Stage 2 — Young Star Field**  
The space the Colossus passed through. New stars forming in its wake. Noise interference: stellar EM vs. the organic Colossus signal. Still free-roaming, full thrust authority. See `stages/stage_02_young_star_field.md`.

**Stage 3 — Ice Moon**  
Environmental rails through ice crevasses and caverns. Thermal seeking for geothermal vents replaces audio as primary tool. Hull temperature is the threat. Ghost_reacher encased in ancient ice. See `stages/stage_03_ice_moon.md`.

**Stage 4 — Volcanic Moon**  
The ice moon's inversion. Tidal heating taken to its extreme: constant eruption, sulfur plains, lava flows. Thermal mechanic reverses — seek cool margins, not warmth. Ghost_reacher preserved in sulfur deposit. See `stages/stage_04_volcanic_moon.md`.

**Stage 5 — Geyser Moon**  
Cryovolcanism. The player enters a geyser plume and is briefly carried upward — away from the Colossus. The stage's ghost_reacher faces outward rather than toward the Colossus. See `stages/stage_05_geyser_moon.md`.

**Stage 6 — Methane World**  
Titan-like moon. Alien landscape that looks almost familiar — methane lakes, methane rain, orange haze. Navigation through methane fog in brief windows of clarity. Ghost_reacher submerged in methane lake. See `stages/stage_06_methane_world.md`.

**Stage 7 — Ocean Interior**  
A waterworld. Convection current descends the player toward a seafloor of high-pressure Ice VII. Pressure is the threat; ghosts provide hull tolerance to go deeper. Ghost_reacher faces upward toward the arriving player — the one encounter where they come from the direction it faces. See `stages/stage_07_ocean_interior.md`.

**Stage 8 — Storm World**  
A planet of permanent violent storm. Environmental rails through discharge corridors to a calm vortex eye where the ghost_reacher waits. Timing mechanic: read the lightning rhythm, move in the gaps. See `stages/stage_08_storm_world.md`.

**Stage 9 — The Rogue**  
A rogue planet with no star. Total darkness, ship lights to twenty meters, a ghost_reacher that has been preserved in vacuum for billions of years and looks new. The oldest ghost_reacher. The quietest stage. See `stages/stage_09_rogue_planet.md`.

**Stage 10 — Ringed Gas Giant**  
Back to open space. Visual gravitational lensing as the primary seeking tool — find where the stars bend. Navigation through ring debris. See `stages/stage_10_ringed_gas_giant.md`.

**Stage 11 — Pulsar**  
A neutron star remnant sweeping radiation on a precise interval. Rhythmic seeking — orient in the gaps between pulses. The most regular environment in the game. See `stages/stage_11_pulsar.md`.

**Stage 12 — Supernova Remnant**  
The interior of a dead star, now expanding nebula. Synthesis stage: all sensing modalities active simultaneously, none sufficient alone. See `stages/stage_12_supernova_remnant.md`.

**Stage 13 — Dark Nebula**  
Dense dust cloud — stars gone, vision gone, audio scattered. Haptic dominant: the rumble pak carries the Colossus's gravitational signature. The last stage before the gravity threshold. See `stages/stage_13_dark_nebula.md`.

**Stage N — Colossus Gravity Threshold (stub)**  
The point where the player realizes they can no longer thrust free. Not a designed event — the player notices when it's already true. Stage number not decided.

**Stage N+1 — Arrival (stub)**  
He is here. The hold. The still center. See `stages/stage_arrival.md`.

---

## Open Questions

- **How many seeking stages?** Many — the seeking mechanic has room to develop across a large number of stages before rails begin. Exact count not decided.

- **Oxygen scope:** Persists through all stages until arrival. At the arrival stage the vessel is shed — the player no longer needs oxygen because they are no longer in a ship. The oxygen mechanic ends not through depletion but through transformation. See `mechanics/oxygen.md`.

- **Stage length:** Targeting 10–20 minutes per stage for a first visit. Return visits shorter — the player knows the space. Complete first playthrough roughly 45–90 minutes total.

- **What does completing a stage feel like?** Arrival, not victory. No fanfare, no score. Something changes — the next stage opens — and that is all.

- **How explicit is the ghost narrative?** The player finds a frozen figure at the beacon. They are not told it is a previous seeker. They can infer — or not. This is consistent with the game's philosophy of holding questions open.
