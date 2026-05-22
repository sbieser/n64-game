# Cockpit — Visual Language

*The player is inside the ship. Not behind it. The cockpit is the frame through which the entire game is experienced.*

---

## The Decision

The game is about a person — small, exposed, alone in the cosmos. Third-person puts you behind that person. Cockpit puts you inside them. You don't watch a seeker. You are the seeker.

Every stage, every Pioneer, every ghost ship tumbling past — all of it is seen through this window. The cockpit is not a HUD layer on top of a game. It is the game's primary visual fact.

---

## Reference

Three images from *2001: A Space Odyssey* define the design language:

**The pod interior (overhead shot):** The dashboard wraps in a curved arc following the spherical hull. The visual language is small clustered indicator lights — organized grids of backlit squares. Cyan/teal for nominal systems. Red for warnings. Amber for active. A few small recessed monitors. Dark everywhere except the instruments. This is the UI language we adopt.

**The pod side view:** A circular porthole. Cramped, red ambient fill, the astronaut large in frame. This tells us what NOT to do — the porthole is too restrictive. We cannot cut to exterior shots the way Kubrick could. We need to see space.

**The space liner cockpit (wide shot):** This is the template. A wide horizontal windshield spanning the full screen width. The pilots are dark silhouettes against the void. The space station fills the view — the outside is the dominant element, not the interior. Dashboard below with amber button rows. Screens recessed at the bottom corners. Dark interior, bright space beyond.

---

## Composition

The screen is divided into two zones:

```
┌─────────────────────────────────────┐
│                                     │
│                                     │
│          THE VOID  (~60–65%)        │
│       (3D scene shows through)      │
│                                     │
│                                     │
├──────────────────┬──────────────────┤
│  [indicators]    │  [screen] [hand] │
│                                     │
│        DASHBOARD  (~35–40%)         │
│                                     │
└─────────────────────────────────────┘
```

**Narrow side pillars** frame left and right edges — dark, just wide enough to feel enclosed. You are inside something.

**The windshield** is a wide horizontal band. Not a porthole — a panorama. The void is big. The cockpit is small inside it.

---

## Dashboard

Dark panel. The only light comes from the instruments. No overhead lighting. The void outside is brighter than the interior.

**Ambient fill:** Deep red-black. Instrument lights are the color source. This matches the emotional register — intimate, pressured, alone — and echoes the pod interiors of *2001*.

**Primary UI — indicator clusters:**

Instrument state is communicated through small backlit square buttons arranged in organized grids, exactly as in the *2001* pod. No bars. No numbers by default. Pure color language:

| Color | State |
|---|---|
| Cyan / teal | Nominal |
| Amber / orange | Degrading |
| Red | Critical |
| Dark / off | Dead / absent |

The **oxygen meter** is a cluster of 8–10 small squares. Full oxygen: all cyan. As oxygen depletes, squares go dark from right to left. Critical: the remaining squares go red. No number is needed. You know at a glance.

The **hull temperature** (ice moon, volcanic moon) replaces or augments the oxygen cluster depending on the stage. Cold stages: squares shift toward deep blue as hull cools. Hot stages: squares shift toward red as hull heats. The cluster changes meaning by stage without changing form.

**Secondary UI — signal screen:**

One small recessed monitor on the dashboard. The signal direction indicator from `mechanics/seeking_mechanic.md` lives here — a moving dot or needle showing horizontal and vertical direction to the signal source, intensity encoded in dot size. The player glances down at it the way a pilot glances at a compass. It is never the primary instrument. The ears are primary. This is the fallback.

**The hand:**

One gloved hand visible, resting near controls at the bottom corner of the dashboard. Present, physical, human. No animation needed beyond idle. Its only purpose is to make the player feel embodied — you are not a camera. You are a person.

---

## Color and Lighting

The cockpit interior is dark. The instruments are the light source.

**Interior palette:**
- Structural surfaces: near-black, slightly warm (not cold grey — there is a person in here)
- Instrument lights: cyan, amber, red — the 2001 button language
- Ambient fill: deep red-black, lit from below by the dashboard

**Space outside:**
- Cold. Blue-white star field. 
- Color temperature shifts across the journey — cold blue in early stages, warming toward amber and orange as the Colossus is approached
- The contrast between cold void and warm interior keeps the cockpit feeling like shelter

The cockpit should feel like the warmest place in the universe — not because it is warm, but because it is the only enclosed thing in an infinite cold.

---

## What Changes by Stage

The cockpit frame and hand are constant. What changes is:

- **Which indicator cluster is active** — oxygen in space stages, hull temperature on moon stages, hull pressure in the ocean, hull integrity in the storm
- **The color temperature of the void outside** — cold blue early, warming late
- **Visibility through the windshield** — full stars in space stages, rushing ice walls on the ice moon, complete black in the dark nebula

**The dark nebula (Stage 13):** Nothing visible through the windshield. Pure black. The dashboard instruments are the only light in the scene. The player is navigating by rumble pak alone. The cockpit feels like a sealed box. This is the most isolated the player will feel before arrival.

---

## The Vessel Falls Away

At the Colossus arrival, the cockpit overlay fades.

The dashboard lights go dark one by one. The side pillars dissolve. The hand is gone. The windshield frame disappears. What was the border between you and the void is no longer there.

The player is now just a point of view — no readouts, no instruments, no frame — looking at something incomprehensible.

This moment only works because the cockpit was there before it. The intimacy of the dark interior, the small instrument lights, the gloved hand — all of it made the cockpit feel like shelter. Its removal is the game's most significant visual event. The seeker has become something that doesn't need a ship.

---

## N64 Implementation Notes

The cockpit is a **2D overlay layer** drawn after the 3D scene via rdpq. The 3D scene renders into the framebuffer first — the void, the stars, the stage geometry. Then the cockpit frame and dashboard are drawn on top as 2D quads. The windshield area simply has no overlay — the 3D scene shows through.

**Cost:** Essentially free. The overlay is a small number of colored quads at fixed screen positions. The indicator light clusters are small quads with color state driven by game variables. No textures required — solid-colored geometry is the design.

**The fade-out:** The cockpit overlay's alpha is driven by a single float. Fade to zero over a few seconds and it's gone. No special rendering required.

**FOV:** First-person cockpit benefits from a slightly narrower field of view than third-person. Current value is 70°. Target is 60°–65° — narrow enough to feel like looking through a windshield, wide enough that the void still overwhelms.

---

## What This Changes in the Docs

The DESIGN.md description of the player character — "a small human figure traveling through space, small, exposed, vulnerable" — remains emotionally true but is no longer literally visible. The player is that person. They are not seen from outside. The cockpit is the only representation of the player's physical presence. Update `DESIGN.md` to reflect first-person perspective.

Stage atmosphere sections in stage docs were written imagining a third-person camera. Each will need revisiting — geometry scale, what the player actually sees, draw distances — all change when the viewpoint is inside the cockpit rather than behind a ship.
