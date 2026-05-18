# Controls

*Global mechanics. These apply across all stages. What changes between seeking stages and rail stages is not the controls themselves — it is whether the player's thrust is the primary mover, or whether the Colossus's gravity has taken over.*

---

## Ship Movement

- **Analog stick:** yaw (left/right) and pitch (up/down) — turns the ship, changes heading
- **No roll.** The world-up vector is locked. The camera always knows which way is up regardless of ship attitude. This prevents disorientation and keeps audio navigation readable.
- **Traversal is fully 3D.** Heading is movement direction. The player pitches toward something above or below and thrusts to reach it. No separate vertical thrust buttons needed.

---

## Thrust

Rotation is free. Thrust is the consequential act.

The ship does not move forward unless the player chooses to thrust (in seeking stages). Commitment to direction comes before commitment to movement.

- **A button:** thrust forward in the facing direction
- **A button held:** boost — faster forward thrust

The player can yaw and pitch indefinitely. They can stand still and listen. See `oxygen.md` for how movement state affects oxygen depletion.

---

## The Transition to Rails

In seeking stages, the player chooses to thrust. Forward motion is a decision.

As the Colossus's gravity takes over in later stages, a pull begins. At first barely perceptible — a drift the player can't account for. Then stronger. Then the player's thrust is minor against it. At full rails, the player no longer thrusts. They are being carried by physics. They can only steer within what the gravity does.

This is not a game mechanic imposed from outside — it is the universe asserting what gravity asserts near something massive enough. The player crossed the threshold during the seeking stages. They find out when their thrust stops mattering.

**In code:** a `control_authority` float governs this. 1.0 = full player thrust control, 0.0 = full gravity pull. Player input and gravity fill complementary portions of forward motion. This float slides from 1.0 toward 0.0 as the player moves through the later stages.

---

## What the Player Never Has

- **Roll.** The world-up vector is always locked.
- **Reverse thrust.** The player can reorient and thrust the other direction, but there is no dedicated backward button.
- **Stop.** In the rail stages, the player cannot halt. Gravity has them.
