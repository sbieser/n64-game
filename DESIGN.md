# The Colossus — N64 Rails Game
## Design Brief & Claude Code Context

---

## What This Project Is

A rails-based 3D space traversal game for real Nintendo 64 hardware, built with **libdragon** (preview branch) and **tiny3d**. The player travels through deep space on a fixed forward path — obstacle survival, not combat — searching for a mythic cosmic entity known as The Colossus. When found, the player becomes one.

Emotional touchstones: **Akira** (transformation as terrifying transcendence), **The Fountain** (lone traveler moving toward something sacred), **Passage by Jason Rohrer** (mortality, repetition, the finite journey — spirit not mechanics). Think meditative, inevitable, cosmic.

---

## Narrative

You are a seeker. Small, fast, propelled through the cosmos toward something incomprehensibly vast. The Colossus is out there — a being of such scale that worlds die in his wake. You don't know what you'll find. You don't know what you'll become.

The song that inspired this game asks: *"Do I dare to disturb the universe? Lest I become he the Colossus."* That tension — the cost of seeking, the fear of arrival — is the emotional core. **No actual song or lyrics are used in the game.**

---

## Three-Act Structure

### Act 1 — The Search
- Fast, almost frantic rails movement through deep space
- Player feels small and purposeful
- Environment: debris fields, dying stars, strange geometry
- Hints of the Colossus: his silhouette against a nebula, gravitational distortions, dead worlds in his wake
- Music: original, urgent, searching

### Act 2 — His Gravity
- Movement slows subtly — you're being pulled now, not just flying
- Rails begin to curve toward him
- Environment grows stranger, more sparse
- Encounters: frozen seekers who didn't make it — people, drifting, still reaching forward (see Ghost Mechanic)
- Music: sparser, the silence gets louder

### Act 3 — The Colossus
- You find him. He is enormous. The game slows to near stillness.
- The vessel falls away — the final approach is the person alone, unprotected, traveling as themselves
- No combat. You don't fight him. You approach.
- The player figure merges with the Colossus geometry — not violently, not triumphantly. Simply becoming part of something already ancient, already moving.
- The screen does not go black. The Colossus continues through space. Eternal or dead — you can't tell. You're part of it now.
- Music: crescendo, then something unresolved — not a conclusion

**CRITICAL — DO NOT RESOLVE THE AMBIGUITY:**
The ending must not answer whether becoming the Colossus is death or transcendence, triumph or tragedy. This is intentional and load-bearing to the entire experience. Does the Colossus even know if it's alive or dead? Does scale make that distinction meaningless? The transformation sequence should make the question feel unanswerable — and somehow okay. Visually, tonally, and in code: hold it open. Never tip toward triumph. Never tip toward tragedy. Just vastness.

---

## Core Gameplay

**Genre:** Rails traversal / obstacle survival (Star Fox 64 locomotion, not Star Fox 64 combat)

**Player character:** A person — a human figure traveling through space. Not a pilot in a cockpit, not a ship. Small, exposed, vulnerable. This creates identification not distance. In Acts 1 and 2 they travel within some kind of vessel; in Act 3 the vessel falls away and they continue alone.

**The loop:**
- Player moves forward automatically on a fixed rail through 3D space
- Analog stick steers left/right/up/down within a bounded play area
- Obstacles must be avoided, not destroyed
- Survival = staying alive long enough to reach The Colossus

**Obstacle philosophy — the universe is indifferent, not hostile:**
- Gravitational fields that pull the player off course — player fights the drift
- Debris from dead worlds the Colossus has already passed through
- Ghost seekers (see Ghost Mechanic) — environmental storytelling + possible hazard
- Cosmic phenomena: pulsar waves, collapsing stars — timed movement windows
- Increasing exposure/degradation as you approach — the journey itself consumes you

**Pacing rhythm:**
- Flow → disruption → flow again
- Early: chaotic, fast
- Mid: sparse, strange, dreamlike
- Late: obstacles fall away, you're just moving toward him

**No shooting mechanic.** No score. Life depletes on obstacle contact — death is possible and meaningful.

---

## Ghost Mechanic

When the player dies, their position along the rail, their lateral/vertical offset, and their run index are written to **SRAM** on cartridge.

On subsequent runs, those positions are loaded and rendered as frozen figures — people, drifting in the exact spot and posture they died, still reaching forward. They become part of the environment of Act 2, growing denser closer to The Colossus.

- They are the player's own death history made visible
- They are also environmental storytelling — other seekers who came before
- Their effect on gameplay (purely cosmetic vs soft hazard) TBD — iterate
- Cap stored deaths at a reasonable number given SRAM limits (TBD, ~5–10)
- Ghost figures should feel still and sad, not threatening

---

## Visual Direction

**Aesthetic:** Lo-fi N64 polygon style embraced fully — not apologized for. The hardware constraints *are* the art direction. Think how 2001: A Space Odyssey feels sparse and overwhelming — that's the target emotion, achieved through simplicity not detail.

- Deep space: stark star fields, shifting color temperature as player approaches Colossus
- The Colossus rendered as simple but massive geometry — overwhelming by scale, not complexity
- Player character: a small human figure — minimal geometry, recognizably a person
- Color palette: deep blacks, cold blues and purples in Act 1 → warm cosmic oranges and whites in Act 3
- tiny3d's GLTF pipeline for models (via Blender + Fast64 materials)

**Reference:** StarStrike 64 proved a rails shooter is achievable in libdragon/tiny3d. Study its approach.

---

## Audio Direction

**No licensed music is used.** All audio is original.

- Ambient generative-feeling soundtrack that shifts with game state
- libdragon supports **Opus audio compression** for compressed music streaming
- Sound design prioritizes space, silence, and scale — not action game energy
- The transformation sequence in Act 3 is the audio climax

---

## Technical Stack

| Component | Tool |
|-----------|------|
| SDK | libdragon (preview branch) |
| 3D rendering | tiny3d |
| 3D modeling | Blender + Fast64 materials → GLTF |
| Emulator | Ares (Homebrew mode on) — primary dev target |
| Secondary emulator | Gopher64 (performance testing) |
| Build system | Make + Docker toolchain (libdragon official image) |
| Language | C (primary), with tiny3d C API |
| Audio | libdragon audio + Opus for music streaming |

**tiny3d notes:**
- Requires libdragon **preview branch** specifically
- Models and matrices are DMA'd into RSP each frame — keep both in RDRAM while rendering
- Use GLTF import pipeline with Fast64 custom properties enabled
- Interops directly with libdragon's RDPQ API

**Pyrite64:** Watch this project — it's a visual editor + toolchain manager built on libdragon/tiny3d by the same author (HailToDodongo). Currently WIP/early dev but may become useful for scene authoring.

---

## Project Priorities

1. Get a basic rails movement loop working first — player figure moving forward, analog stick steering, collision with a simple obstacle
2. Establish the visual tone early — star field, camera, player character model
3. Add ghost mechanic — SRAM write on death, load and render frozen figures on next run
4. Add obstacle variety incrementally
5. Audio last, or in parallel once gameplay loop is stable

---

## What Claude Code Should Know

- This is a **side project** with limited time — weekdays light, weekends lighter (family commitments)
- Prefer **simple, working, incremental** code over ambitious incomplete systems
- Always explain N64-specific constraints when they affect implementation choices
- Keep the emotional vision in mind: **meditative, inevitable, cosmic** — not an action game
- When in doubt, do less and do it well

---

## Design Philosophy — Questions This Game Does Not Answer

These are open intentionally. Do not resolve them in code, visuals, audio, or text:

- Is becoming the Colossus death or transcendence?
- Is the Colossus alive or dead — does it even know?
- Are the frozen seekers who didn't reach him failures, or did they arrive differently?
- Does scale make the distinction between life and death meaningless?

The player should carry these questions out of the game with them. Any implementation decision that accidentally answers one of them should be reconsidered. The game's job is to make the questions feel vast and okay, not to resolve them.
