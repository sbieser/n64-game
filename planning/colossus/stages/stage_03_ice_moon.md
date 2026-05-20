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

## The Ghost_Reacher

At the stage's end, past a long descent through the ice, the ghost_reacher is frozen into a cavern wall. Not near the surface — deep, where the ice is ancient and layered. The figure still faces forward, still reaching. But it is encased. The ice grew around it across an immense span of time.

This ghost_reacher is vastly older than Stage 2's. The ice grew around it. That takes geological time. The player understands something about scale here — not spatial scale, but temporal. Someone was here before any living human's reckoning.

The signal from this ghost_reacher is muffled by ice. The player must move close to read it clearly. When they do: the signal is warm. The ghost_reacher's reactor is still producing a faint thermal signature after all this time. The warmest thing on this moon. Preserved by the same cold that would kill anyone who came unprepared.

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

- The gas giant fills the upper sky — banded storms visible, enormous, static. It does not acknowledge you.
- Geothermal vents: faint heat shimmer over dark ice. Not dramatic. The kind of detail you miss if you're not looking for it.
- Ice cavern interiors: near-total darkness. Ship lighting reaches ten meters. The cavern continues past that.
- The cosmic signal: present but muffled, lower in the mix than it has been. Not gone — just changed, filtered by ice and magnetosphere.
- Sound: the creak of ice settling. Something geological, very far below.
- Color: cold blue-white ice against the warm amber-orange of the gas giant above. The only warmth in the sky is the planet that's killing you slowly with its tidal forces.

---

## Open

- Exact thermal depletion rate and vent frequency — needs playtesting against felt tension
- Whether the hull temperature indicator is audio (a low frequency hum that drops in pitch), visual (color shift at screen edges), haptic (rumble pak), or a combination
- What the player sees at the far end of the stage that transitions to Stage 4
