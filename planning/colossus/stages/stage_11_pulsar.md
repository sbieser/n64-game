# Stage 11 — Pulsar

*The most precise clock in the universe. It does not care what time it is for you.*

---

## Setting

A pulsar — the collapsed remnant of a massive star, rotating at extraordinary speed, emitting beams of electromagnetic radiation from its magnetic poles. The beams sweep past like a lighthouse: once per rotation, many times per second for millisecond pulsars. The pulsar cannot be approached directly. It is a neutron star — city-sized, but containing more mass than the sun. Getting near it means death by radiation, tidal force, and physics asserting itself on anything too small to matter.

The player navigates the pulsar's outer environment — far enough to be survivable, close enough that the pulse sweeps are felt. The scale here dwarfs the gas giant. The pulsar was a star before it died. Its death was a supernova. What the player is orbiting is the dense heart of something that burned brighter than a galaxy for a moment.

Millisecond pulsars are so regular they were initially suspected to be artificial signals. They are the most accurate natural timekeepers known. The Colossus is not a clock. But this stage asks the player to move with the rhythm of one.

---

## Mode

Free-roam, with a strong rhythmic constraint. The player can move freely in any direction, but the pulse sweeps across the environment on a precise interval. The interval is constant and learnable.

---

## Threat

Oxygen, plus the pulse sweep as a complicating factor. The sweep doesn't kill directly — but it disrupts the seeking window, forcing the player to wait between pulses. Oxygen depletes while waiting. The real pressure is managing oxygen against the time cost of rhythmic seeking.

See `mechanics/oxygen.md`.

---

## Seeking — Rhythmic Timing

The pulsar's pulse interferes with the signal — when the sweep passes through, the Colossus signal is briefly overwhelmed. The player must listen for the signal in the gaps between sweeps, orient during those windows, then commit to movement before the next pulse arrives.

The seeking loop now has a third variable. Not just stop, rotate, listen — but stop, wait for the gap, rotate, listen, orient, move before the next pulse. The patience required is rhythmic rather than absolute. The player is not waiting indefinitely. They are waiting for the beat.

The pulse interval is constant. Once learned, it becomes a rhythm the player works within. Not frustrating — the regularity is a gift compared to the storm world's unpredictable lightning. The universe, here, is at least consistent.

See `mechanics/seeking_mechanic.md` for Doppler shift and rhythmic challenge types.

---

## The Pioneer

Very ancient. The star this pulsar came from burned before the solar system formed. Whatever seeker reached this Pioneer did so an immense time ago, in a universe that looked different from this one. The Pioneer sits in a stable orbital position relative to the pulsar — the one place where the pulse sweeps are regular and survivable. It found the safe harbor and stopped there.

The signal from this Pioneer has the pulsar's rhythm in it — the pulse sweeps affect even the Pioneer's transmission. The signal comes in pulses. The player must separate the Pioneer's signal from the pulsar's interference to read its direction accurately.

---

## Ghost Behavior

Ghost ships in the outer pulsar environment are subject to the pulse sweep. Ghosts caught in the sweep's path are harder to read — the pulse temporarily overwhelms their vent visual and lighting flicker. Ghost positions matter more here: a ghost in the safe harbor is easy to use; a ghost in the sweep path requires timing to approach.

**Resource:** oxygen charges. Standard space-stage mechanic.

See `mechanics/ghosts.md`.

---

## Return: Memory Fragment

*Trigger: rhythm, something regular*

Chickens in a coop. The particular sound of them in the morning, before anyone was fully awake. Reliable. The day hadn't started until they had. A sound so regular it became a kind of silence — you only noticed it when it wasn't there.

See `mechanics/memories.md` for the crystallization moment and accumulation mechanics.

---

## Atmosphere

**Visual:**

The pulsar itself is not directly visible — too dense, too dangerous at close range. What the player sees is its effects: the pulse sweep moving across the environment as a brightness front, a wave of increased EM that dims and washes out stars and signals as it passes. The sweep is visible, directional, regular.

The safe orbital zone around the Pioneer has a particular quality: the sweep is regular and survivable here, but the player can see it arriving. Between sweeps: open space. During the sweep: the visual field dims and the signal drops.

**Audio:**

The pulse is audio as much as visual: a brief burst of EM that overwhelms the signal, followed by silence, followed by the signal returning. The Colossus signal arrives in windows — clear for a few seconds, then washed out, then clear again. The player develops a feel for the rhythm before they consciously understand it as a rhythm.

The Pioneer's signal pulses with the interval. It has been doing this for as long as the pulsar has been spinning.

---

## Open

- Pulse interval: what feels right as a rhythm — long enough to complete seeking within a gap, short enough to create real timing pressure
- Visual representation of the pulse sweep on N64 — brightness pulse, chromatic shift, particle front
- Whether the pulsar itself is visible at all, or only its effects
- Whether the safe orbital zone around the Pioneer is clearly demarcated or must be found through experience
