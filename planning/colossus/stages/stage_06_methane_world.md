# Stage 6 — Methane World

*It looks almost right. That is the strange part.*

---

## Setting

A large moon with a thick nitrogen atmosphere and surface lakes of liquid methane. Methane rain. Methane rivers. Shorelines. From the ship, it looks familiar — the geometry of a world with weather and water and coast. But nothing is water. The orange haze of the atmosphere filters what little starlight reaches here. The sun is a dim smudge through the haze, far enough away to be barely worth the name.

The familiar geometry of a world with weather and liquids, but wrong. This is the uncanny valley of planetary bodies. Lakes that look like lakes but are not. Rain that sounds like rain but is methane. Shore that looks like shore but you cannot walk on it without a ship. The wrongness is precise and unannounced. The game does not tell you what is wrong. You notice it, or you don't.

---

## Mode

Environmental rails through the atmosphere and across methane terrain. Thick atmosphere wind patterns, methane fog banks, and the geography of an alien coast define the navigable channels. The rail is not walls — it is navigable air and the coastline's shape.

---

## Threat — Visibility

No dangerous heat, no cold, no crushing pressure. The threat here is navigation: **methane fog**. The atmosphere is dense with it. The cosmic signal is heavily attenuated — fog and atmosphere scatter it. In the thickest fog banks, visibility drops to near zero.

The mechanic: find fog-free corridors. Navigate by brief windows of clarity. Orient during the clear moments, commit to direction before the fog closes again.

This is the same patient orientation discipline as the void — stop, rotate, listen, orient, move — but instead of empty space, the player is inside an alien atmosphere they cannot see through.

See `mechanics/oxygen.md` — oxygen is the operative resource here; this stage uses no separate planetary threat mechanic.

---

## Seeking

The fog forces the same patient discipline as Stage 1's void: the signal is present but attenuated, and the player cannot move blindly.

The ghost_reacher's signal is the primary navigation tool here. Methane does not block it the way ice does — the signal from the partially submerged ghost_reacher at the stage's end carries clearly through the fog. The player uses it the way they used the Stage 1 beacon: as a heading to orient toward during windows of clarity.

**Stop → Rotate → Listen → Find the ghost_reacher's signal through the fog → Move → Stop when the fog closes**

The seeking loop runs slower here than in open space. The windows of clarity constrain when the player can orient, not just where they can go. Patience applied under different constraints.

See `mechanics/seeking_mechanic.md`.

---

## The Ghost_Reacher

Partially submerged in a methane lake at the stage's end — sitting in it, arm above the surface, reaching. The methane has coated it in orange residue over an immense span of time. It is almost beautiful. It will be there until something changes this moon at a geological scale.

The signal from this ghost_reacher carries clearly — methane doesn't block it the way ice does. The player can detect the ghost_reacher from further away than usual, which makes navigation through the fog possible: the signal grows stronger as the fog thins, giving direction even when vision gives nothing.

---

## Ghost Behavior

Ghost ships on the surface or in the methane terrain persist normally, coated in orange haze residue over repeated visits. The fog makes locating them harder — a ghost could be meters away and invisible.

**Resource:** oxygen charges. The methane atmosphere is alien but the ship is still a ship. The oxygen mechanic continues as in the void stages — this moon has no breathable air, but the ghost's oxygen supply is unchanged.

See `mechanics/ghosts.md`.

---

## Return: Memory Fragment

*Trigger: crossing into somewhere almost-familiar*

A bridge over the river into the neighbor's yard. Horses on the other side. Rabbits. Two women's long friendship making a path between their properties — crossing it meant being in someone else's world that was also somehow home. The horses knew you. The rabbits did not. You crossed it so many times that crossing it felt like a word you'd said until it lost meaning, and then found again.

See `mechanics/memories.md` for the crystallization moment and accumulation mechanics.

---

## Atmosphere

**Visual:**

Orange. The haze is omnidirectional — it has no source, comes from no direction, simply exists as the quality of light here. Everything is slightly amber, slightly diffuse, slightly wrong.

The methane lakes are the stage's spatial anchors: their surfaces reflect the haze-filtered sky in a way that reads as water at first glance and as something alien on the second. The shorelines are real — they are just made of the wrong things, and the liquid is the wrong liquid. The geometry of coast is deeply familiar. Nothing about the coast is familiar.

Methane rain falls during parts of the traversal — a heavier sound than water, lower-pitched. Visible as darker streaks through the haze.

**Audio:**

The haze muffles everything. The cosmic signal is quieter here than it has been since Stage 1. The ghost_reacher's signal is the dominant navigational sound — clearer, warmer, cutting through what the atmosphere scatters.

Methane rain has a distinct audio character: slower drops, heavier impact, a wetter sound than water rain somehow. The wind through the atmosphere carries a low continuous moan.

---

## Open

- How to represent methane fog on N64 — distance fog, color shift, reduced draw distance
- Whether the methane lake is visible at the stage's end or only the ghost_reacher's reaching arm above the surface
- Whether any bioluminescence exists in the methane — speculation, but a visual possibility
- Sound design: how different methane rain needs to be to read as wrong
