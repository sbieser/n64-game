# Stage 3 — Ice Moon

*The signal grew fainter inside the magnetosphere. Something else became clearer.*

---

## Setting

An ice moon orbiting a gas giant somewhere in the Colossus's wake. The Colossus passed through this system an immense time ago — long enough for the moon to have settled into deep cold, the ice shelf to have fractured and refrozen across millennia. Beneath the surface: geothermal activity from the gas giant's tidal kneading. The moon is not dead. It is cold at the surface and warm, very far down.

The player descends through crevasses and ice caverns toward that warmth. The scale is planetary but intimate compared to the void — the sky is close, the gas giant fills the upper quadrant, enormous and banded and indifferent. Smallness here is different from the smallness of infinite space. This is the smallness of a body on a surface, inside a landscape that was here long before anyone came looking.

---

## Mode

Environmental rails. Not Colossus gravity — the moon's own gravity and the ice terrain channel the player through crevasses, caverns, and surface runs. The path is defined by the landscape. Forward motion is assisted: the slight downslope of the ice shelf, the channeling geometry of ice walls, the pull of a gravitational body with nowhere else to go but down.

The player steers within the channel. They cannot stop. Unlike the Colossus gravity rails later in the journey, they are not being pulled against will — they are navigating a landscape that simply doesn't offer many directions.

---

## Threat — Hull Temperature

Oxygen is not the operative mechanic here. The hull is cold. This moon is cold. The suit's thermal management fights a constant battle against vacuum-temperature ice and the absence of sunlight.

**Hull temperature** drops steadily. Ice cavern interiors are colder than surface crevasses. Dark passages are colder than areas near geothermal vents. Moving fast moves through cold faster.

At zero: the ship stops. The cold wins. Ghost state.

See `mechanics/ghosts.md` for how the ghost appears here and what it provides. See `mechanics/oxygen.md` for the planetary equivalent table — hull temperature is this stage's form of the same indifference.

---

## Seeking — Thermal Navigation

The cosmic signal is attenuated inside the moon's magnetosphere and ice mass. The player can still faintly hear it — it hasn't vanished — but it is no longer the primary navigation tool. It is muffled, quieter, present but not dominant.

What is detectable: heat. The geothermal vents emit infrared radiation the ship's hull senses. Warmth means a vent. A vent means thermal replenishment.

The **thermal seeking mechanic** applies the same loop to heat rather than the Colossus signal:

**Stop → Rotate → Feel → Find warmest direction → Move toward it**

The hull warms when facing warmer regions. Cold directions feel uniformly cold. The player learns to read thermal gradient the way they learned to read audio volume — directional, patient, the same cost for moving wrong.

This is not a new mechanic. It is the same mechanic applied through a different sense. See `mechanics/seeking_mechanic.md` for the full seeking loop and the thermal variant documented there.

---

## The Pioneer

At the stage's end, past a long descent through the ice, the Pioneer is frozen into a cavern wall. Not near the surface — deep, where the ice is ancient and layered. The figure still faces forward, still reaching. But it is encased. The ice grew around it across an immense span of time.

This Pioneer is vastly older than Stage 2's. The ice grew around it. That takes geological time. The player understands something about scale here — not spatial scale, but temporal. Someone was here before any living human's reckoning.

The signal from this Pioneer is muffled by ice. The player must move close to read it clearly. When they do: the signal is warm. The Pioneer's reactor is still producing a faint thermal signature after all this time. The warmest thing on this moon. Preserved by the same cold that would kill anyone who came unprepared.

---

## Ghost Behavior

Player death ghosts freeze in place at the exact position of hull-zero. The cold stopped them there. On the next visit the wreck is exactly where it happened — encased in fresh ice if the gap between visits was long, still bare metal if recent.

The ghost's reactor still produces residual heat: a **thermal charge**. Approaching and drawing from it warms the hull, buying more traversal before the next vent. The charge system is the same as oxygen (5 charges, SRAM-persisted, once per visit). The resource is warmth rather than air.

Ghosts here don't accumulate into fields. They appear where the cold won — in the stretches furthest from vents, in the darkest passages, at bends in the crevasse where the stage's particular threat is sharpest. Not testimony pointing toward the goal. Evidence pointing at where to pay attention.

---

## Return: Memory Fragment

*Trigger: specific cold, a particular winter*

The river in winter. Everything stiller. The deer still came to the bank at certain hours — they didn't stop because of the cold, they just moved differently. Cold in a specific way, the cold of that particular property and no other, the way the morning felt before the sun cleared the hill.

See `mechanics/memories.md` for the crystallization moment and accumulation mechanics.

---

## Atmosphere

- The gas giant fills the upper portion of the windshield — banded storms visible through the glass, enormous, static. It does not acknowledge you.
- Geothermal vents: faint heat shimmer over dark ice visible ahead through the windshield. Not dramatic. The kind of detail you miss if you're not looking for it.
- Ice cavern interiors: near-total darkness. Ship lighting reaches ten meters through the windshield — the ice walls rushing toward you from the dark. The cavern continues past that.
- The cosmic signal: present but muffled, lower in the mix than it has been. Not gone — just changed, filtered by ice and magnetosphere.
- Sound: the creak of ice settling. Something geological, very far below.
- Color: cold blue-white ice against the warm amber-orange of the gas giant above. The only warmth in the sky is the planet that's killing you slowly with its tidal forces.
- Cockpit: the oxygen indicator cluster is replaced by hull temperature. Eight squares — all lit cyan at stage start. As temperature drops, the squares darken and shift toward deep blue from right to left. At critical cold, the remaining squares pulse red. The player glances at it the way a driver glances at a temperature gauge.
- The Pioneer is embedded in the right-hand wall of the cavern. It appears on the right side of the windshield as the player approaches — frozen into the rock, arm still reaching forward, framed by the cockpit pillar.

---

## Production Layout

Concrete geometry and placement spec. All measurements in world units. 1 world unit ≈ 1 meter. Player ship: ~2 units wide. Forward speed on environmental rails: ~7 units/second.

**Total path length:** 5,000 units. At 7 units/second this is ~12 minutes at first traversal. No DFS streaming needed — load all geometry at stage start, use draw distance to cull.

---

### Zone 0 — Surface Crevasse (Z: 0 to −1,200)

Open slot in the ice shelf. Narrow sky overhead, gas giant visible. The stage begins here.

**Dimensions:** 60 units wide, open top, ~40 units of visible wall height per side.

**Terrain chunks (wall slabs, each 200 units long, ~40 tris):**
- Wall sections: 6 per side × 2 sides = 12 wall chunks
- Floor sections: 6 floor chunks
- **Zone 0 terrain total: 18 chunks**

**Draw distance for terrain chunks:** 800 units (3–4 chunks visible ahead at any time).

**Placed objects:**
- Gas giant billboard: 1 large translucent quad (500 × 400 units), Y +200, always facing camera. Color: warm orange-amber (0.90, 0.55, 0.18). Draw distance: 3,000 units — visible from anywhere in zone 0.
- Geothermal vents: 3 (cone geometry, ~20 tris each, warm amber glow). Placed at Z −200, −600, −1,000. Draw distance: 300 units.
- Ice pillar formations: 2 (decorative vertical spires, ~30 tris each). Placed at wall edges, Z −400 and −900. Draw distance: 400 units.

---

### Zone 1 — Upper Cavern (Z: −1,200 to −2,400)

Ceiling begins to close. The sky narrows and disappears. The gas giant is no longer visible. The space becomes intimate.

**Dimensions:** 50 units wide, ceiling descends from open to 25-unit clearance over this section.

**Terrain chunks:**
- Wall sections: 6 per side = 12 wall chunks
- Ceiling arch sections (ceiling closing): 6 chunks (each a curved panel, ~50 tris)
- Floor sections: 6 floor chunks
- **Zone 1 terrain total: 24 chunks**

**Placed objects:**
- Geothermal vents: 2 (Z −1,500 and −2,100). Draw distance: 300 units.
- Ice crystal cluster: 2 formations (groups of ~5 faceted spires, ~40 tris total each). Z −1,700 and −2,300. Draw distance: 350 units.

---

### Zone 2 — Deep Ice (Z: −2,400 to −4,000)

Enclosed tube. Near-total darkness past ship lighting (10 meters = 10 units). The longest stretch. Cold wins here if vents are missed.

**Dimensions:** 40 units wide, fully enclosed, 20-unit ceiling clearance.

**Terrain chunks:**
- Wall sections: 8 per side = 16 wall chunks
- Ceiling sections: 8 chunks
- Floor sections: 8 floor chunks
- **Zone 2 terrain total: 32 chunks**

**Placed objects:**
- Geothermal vents: 2 (Z −2,900 and −3,600). Spaced farther apart than earlier zones — this is where hull temperature pressure peaks. Draw distance: 300 units.
- Ice crystal cluster: 1 small cluster (Z −3,200). Provides visual landmark in the dark.

---

### Zone 3 — The Pioneer Approach (Z: −4,000 to −5,000)

The tube opens slightly. The ancient figure is ahead — the warmest thing in the stage, its reactor still producing faint heat after geological time.

**Dimensions:** 50 units wide, ceiling rises slightly to 30-unit clearance.

**Terrain chunks:**
- Wall sections: 5 per side = 10 wall chunks
- Ceiling sections: 5 chunks (ceiling rising — wider arch geometry)
- Floor sections: 5 floor chunks
- **Zone 3 terrain total: 20 chunks**

**Placed objects:**
- Pioneer: 1. Positioned at Z −4,700, embedded in the right-hand wall (X +22). Still reaching forward. Draw distance: 300 units. Warm thermal signature — acts as a vent charge source, the best one on the stage.
- Stage exit geometry: 1 archway or natural opening (Z −4,950). 

---

### Full Object Count

| Category | Count | Tris (approx) |
|----------|-------|--------------|
| Terrain chunks (walls, floor, ceiling) | 94 | ~3,760 total; ~12 visible at once = ~480 tris |
| Gas giant billboard | 1 | 2 |
| Geothermal vents | 7 | ~140 |
| Ice pillar formations | 2 | ~60 |
| Ice crystal clusters | 3 | ~120 |
| Pioneer | 1 | — |
| **Total placed objects** | **108** | — |
| **Max tris on screen at once** | — | **~750** |

750 simultaneous tris is 37–75% of the conservative N64 frame budget (1,000–2,000 tris at 30fps). Room for player ship, ghost ships, and effect geometry.

---

### Draw Distance Summary

| Object type | Draw distance |
|-------------|--------------|
| Gas giant billboard | 3,000 units |
| Terrain chunks (wall, floor, ceiling) | 800 units |
| Ice crystal clusters | 350 units |
| Ice pillar formations | 400 units |
| Geothermal vents | 300 units |
| Pioneer | 300 units |
| Ghost ships | 200 units |

---

## Open

- Exact thermal depletion rate and vent frequency — needs playtesting against felt tension
- Whether the hull temperature indicator is audio (a low frequency hum that drops in pitch), visual (color shift at screen edges), haptic (rumble pak), or a combination
- What the player sees at the far end of the stage that transitions to Stage 4
