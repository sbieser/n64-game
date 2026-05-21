# Controls

*Global mechanics. These apply across all stages. Stages are either free-roam, where the player thrusts, or rails, where the environment carries them forward.*

---

## Ship Movement

- **Analog stick:** yaw (left/right) and pitch (up/down) — turns the ship, changes heading
- **No roll.** The world-up vector is locked. The camera always knows which way is up regardless of ship attitude. This prevents disorientation and keeps audio navigation readable.
- **Traversal is fully 3D.** Heading is movement direction. The player pitches toward something above or below and thrusts to reach it. No separate vertical thrust buttons needed.

---

## Thrust

Rotation is free. Thrust is the consequential act.

The ship does not move forward unless the player chooses to thrust (in free-roam stages). Commitment to direction comes before commitment to movement.

- **A button:** thrust forward in the facing direction
- **A button held:** boost — faster forward thrust

The player can yaw and pitch indefinitely. They can stand still and listen. See `oxygen.md` for how movement state affects oxygen depletion.

---

## Rail Stages

Some stages are rails. In these stages the player does not thrust — the environment carries them forward. The geometry of terrain (ice crevasses, volcanic corridors, ocean currents) or the arc of a descent defines the forward motion. The player steers within what the stage provides.

A stage is either free-roam or rails. There is no gradual transition — the mode is the character of the stage, not a mechanic applied from outside. The analog stick still controls yaw and pitch on rail stages, but the player is already moving forward. The consequential act becomes lateral steering, not thrust.

---

## What the Player Never Has

- **Roll.** The world-up vector is always locked.
- **Reverse thrust.** The player can reorient and thrust the other direction, but there is no dedicated backward button.
- **Stop.** In rail stages, the player cannot halt — the environment is moving them forward.
