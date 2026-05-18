# The Seeking Mechanic

*Seeking is the core mechanic of the game's free-roam stages. It runs from Stage 1 through the last stages before rails begin. New complications and challenges are introduced gradually — sprinkled across stages as the spaces demand, not unlocked in a fixed sequence.*

---

## How This Document Relates to Stage Documents

Stage documents describe a place — its setting, its narrative, what's unique about it. They do not restate mechanics.

When a stage introduces a new mechanic, or a new wrinkle on an existing one, that mechanic gets its own file (like this one). The stage document references it. This keeps mechanic documentation in one place and stage documentation focused on what makes each stage distinct.

A stage does not need to introduce anything new. It may simply be a place where existing mechanics are expressed differently — harder noise, a different signal character, a stranger space. The mechanic file covers the possibility space. The stage file describes the specific use.

Future mechanic files follow the same pattern: `controls.md`, `seeking_mechanic.md`, and whatever else the game develops.

---

## The Fundamental Act

3D positional audio navigation. No waypoint. No map. You follow your ears.

**Volume** = distance to source. Louder means closer.  
**Pan** = horizontal direction. Centered means facing it.  
**Pitch shift** = vertical direction. The signal is above, below, or level.

The player rotates freely — yaw and pitch — listening. When they locate the direction, they thrust toward it. When they overshoot, they stop and listen again.

---

## The Seeking Loop

**Stop → Rotate → Listen → Find direction → Thrust**

Rotation is cheap. Thrust costs oxygen. This creates a fundamental strategy: listen carefully before committing to movement. The player who rushes burns oxygen on wrong directions. The player who is patient gets further.

This loop is the game's central act. It applies across all seeking stages. Later stages add complications to it — noise, changed signal character, spatial dead zones — but the loop itself never changes.

---

## The Signal

### Character

The signal is irregular — not mechanical, not a clean beacon pulse. Almost organic. It reads as something alive, or once-alive, rather than a radio transmitter. This is intentional and must be preserved across stages even as the signal changes.

The signal's **ambiguity is the meaning.** Was it intentional? A suit beacon? A warning? Something that has no human analogue? These questions are never answered. Reference: the LV-426 transmission in *Alien* — hold all interpretations simultaneously.

### Evolution across stages

The signal doesn't stay the same as the player gets closer to the Colossus. Its character develops:

- **Early seeking stages:** Simple. Louder = closer. Pan + volume is enough to navigate.
- **Middle stages:** More complex rhythm. Something like meaning, though what it means is never stated. The player learns to recognize the Colossus's signal specifically — distinct from other signals in the space.
- **Late seeking stages:** The signal is everywhere. The Colossus is so vast you can't triangulate a direction because you're inside its field. Seeking changes register — you're no longer finding something, you're navigating within it.
- **Rail stages:** No signal needed. You're already in it.

---

## Challenge Types

These are not unlocked sequentially. They are introduced and varied across stages as each space's character demands. A stage might use one, several, or a combination.

### Noise Interference

Some environments are electromagnetically loud — stellar nurseries, new stars forming, the aftermath of things that burned. The signal is still there but harder to hold against background noise. The player developed their ear in Stage 1's quiet void. Later stages test that skill in interference.

The player must learn to recognize the Colossus's signal quality specifically — something the noise doesn't have. Not told what that quality is. Discovered through experience.

### Signal Character Change

The signal presents differently than the player learned it. Not a false signal — the same signal, but changed by distance, by the space it's passing through, by what it's become this close to its source. The player must adapt their reading without losing the thread.

### Ghost Orientation as Passive Navigation

A field of wrecked ships all facing roughly the same direction is a signal too. Every ghost was a seeker. Their last heading is evidence of where they thought the source was. The player learns to read the graveyard — not as instruction, but as accumulated testimony.

Early visits this is just an interesting pattern. Over many visits it becomes a genuine navigation aid. The dead point the way.

### Spatial Dead Zones

Some areas locally suppress the signal — dense material, competing sources, strange geometry near the Colossus. The player loses the signal entirely and must navigate around the dead zone to re-acquire it. The signal dropping to nothing is its own information: something is here, and it's in the way.

### Multiple Competing Signals

Several sources present at once, none clearly dominant. Ghosts have residual signals — they were following the Colossus too, and something of that seeking remains. Other phenomena in the space may emit. The player must learn to distinguish the Colossus's signal from the noise of everything that has died here looking for it.

### Attention vs. Urgency

Seeking requires patience. Oxygen requires movement. These are in direct conflict. The player who listens carefully before committing gets better information but burns time. The player who moves decisively may be moving wrong.

There is no correct answer. The tension is the game.

---

## The Seeker Becomes the Signal

The player is following a signal left by someone who died. Their ghost broadcasts to future seekers. The player will become that ghost.

At some point in the seeking stages this loop becomes visible — not through instruction, but through noticing. You follow a signal. You find a ghost. The ghost is somewhere you've already been. An echo of your own path, from a previous attempt. You were already broadcasting. Future seekers will follow you as you have followed others.

The seeker and the signal are the same thing. What you pursue, you become. What you find, you will be.

---

## Visual Signal Indicator (Accessibility)

For hearing impaired players, a minimal visual element renders the same information the audio carries. Not an arrow. Not a waypoint. A visual translation — the same signal through a different sense.

A small corner indicator, two axes:
- **Horizontal position:** left/right pan
- **Vertical position:** up/down pitch
- **Dot intensity / size:** signal strength

Centered dot = facing the source. Bright dot = close. When audio and visual both converge, the player knows they're aligned and near.

The signal is still the signal. It just has two forms.

Reference: the audio visualizer from the sound demo scene — that level of simplicity is the target. Unobtrusive. Legible. Not a UI element that belongs to a different, more instructional game.

---

## References

- *Star Maker* (Stapledon) — consciousness navigating toward a source it can't fully perceive
- *Nausicaä of the Valley of the Wind* — reading an environment that appears threatening but is operating on its own logic
- The philosophical sublime (Burke, Kant) — orientation toward something that overwhelms without intent
- SF64 positional audio approach — volume + pan as the primary navigation channel, no map
