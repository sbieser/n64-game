# Stage 7 — Ocean Interior

*No surface visible above. No floor visible below. Just water, pressure, and something at the bottom.*

---

## Setting

A water world — a planet entirely covered by ocean, no solid surface accessible from above. The player descends through water that has never known light below a few hundred meters. No sky. No shore. Just water in every direction, growing darker and heavier as the descent continues.

At extreme depth: Ice VII. Not ice formed by cold — ice formed by pressure. Exotic crystalline ice that exists regardless of temperature when pressure exceeds a threshold. The deep floor of this ocean is a surface of hot ice: solid, warm to the touch, formed by the weight of the ocean above it. The Pioneer is at the boundary where water becomes something water shouldn't be.

---

## Mode

Environmental rails — a convection current draws warmer water downward in this region, carrying the player with it. The current's path is the stage. The player steers within it; the current drives the descent.

The descent is slow. Patient. The current moves, but not fast. This is the most meditative of the planetary stages — the pressure increases gradually, the light fails gradually, and the Pioneer is very far down.

---

## Threat — Pressure

**Hull pressure** accumulates with depth. The ship's tolerance has a limit. Without ghost resources, the player cannot reach the seafloor.

Ghost ships on the descent provide hull reinforcement: drawing from a ghost's pressure compensators temporarily boosts structural integrity, allowing deeper descent. The resource is depth capacity — how far down you can go before the hull fails.

This reverses the oxygen logic interestingly: in space, you extend range by replenishing what's running out. Here, you extend range by borrowing structural tolerance from the dead.

See `mechanics/oxygen.md` for the planetary equivalent table. See `mechanics/ghosts.md` for the pressure compensation charge system.

---

## Seeking

The cosmic signal is present but muted by water — still detectable, confirming the descent direction, but not the primary navigation tool.

The seeking loop here is applied to finding ghost ships at depth. Hull pressure accumulates without warning beyond visual cues; the player must locate ghosts before the hull fails. Ghost ships sink to the lowest point of the current's path, so they appear at depth rather than at the entry point. The player seeks their signals in the deepening dark.

The Pioneer's signal intensifies as the player descends toward it — the correct direction is simply down and toward the center of the current. The seeking challenge is not finding the heading (the current defines it) but managing pressure by finding resources in the dark.

See `mechanics/seeking_mechanic.md`.

---

## The Pioneer

On the seafloor, just above the Ice VII boundary. The high-pressure ice has partially grown up around its lower half — exotic crystalline material, warm but solid, slowly encasing what stopped here. Still reaching upward.

The geometry of the encounter is different from every other Pioneer: the player descends toward it from above, and the Pioneer faces up — toward them. For once, the seeker arrives from the direction the ghost faces. From inside the cockpit, descending toward it, the arm fills the center of the windshield — reaching upward toward the player the way the player has been reaching toward every other Pioneer. The inversion is not explained.

---

## Ghost Behavior

Ghost ships sink to the lowest point of the current's path — they appear at depth, not near the entry point. The pressure has slightly deformed them. Their visual state reflects depth: darker, subtly compressed, debris field closer to the hull as if squeezed inward.

**Resource:** pressure compensation. Same charge system — 5 charges, SRAM-persisted, once per visit. See `mechanics/ghosts.md`.

---

## Return: Memory Fragment

*Trigger: moving water, something always present beneath*

The sound of the river in the backyard. Always there, at the back of everything. Still moving while everything else was quiet. The sound that was there before you woke up and there after you fell asleep. Not loud. Constant. The property's heartbeat.

See `mechanics/memories.md` for the crystallization moment and accumulation mechanics.

---

## Atmosphere

**Visual:**

The descent is a study in light failure visible through the windshield — blue-green diffusing to deep blue to nothing. Below the photic zone, the ship lights are the only illumination: ten meters of visible water ahead, then the dark. The water outside the cockpit presses in from every direction. No surface visible above. No floor visible below. The cockpit has never felt smaller.

Hull pressure cluster replaces oxygen: squares deplete as depth increases — not a resource that replenishes between visits but a running measure of structural tolerance. Drawing from a ghost's pressure compensator temporarily restores squares. The player watches it the way they watch the oxygen cluster in space, but the depletion here is continuous and tied to position rather than time.

The Ice VII boundary is the stage's visual payoff: crystalline exotic ice, geometrically perfect structures formed by pressure rather than cold. It emits a faint thermal glow from below — the hot ice is warm, and that warmth bleeds upward through the crystal structure as dim amber light. The Pioneer stands above it, arm raised, faintly lit from below.

If bioluminescence exists in this ocean's life, it appears in the middle depths — brief, not navigational, the stage's only unexpected warmth before the Ice VII floor.

**Audio:**

The water muffles everything. The cosmic signal is quieter here than anywhere else on the journey except the dark nebula. What fills the audio space is hull stress: low structural sounds, the particular sound of pressure asserting itself on metal. The rumble pak builds in intensity with depth — not discrete events but continuous pressure.

The convection current is audible as a low flow, constant, the sound of being carried.

---

## Open

- Whether the Ice VII floor is visually distinct — crystalline texture, warm glow from below
- How to render deep-ocean darkness on N64 — graduated fog, light falloff curve
- Whether there is any bioluminescence from ocean life
- The hull pressure indicator: audio (hull stress sounds), visual (screen-edge pressure vignette), haptic (rumble pak building in intensity)
