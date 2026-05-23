# Stage 2 — The Wake

*Something enormous passed through here. You are moving through what it left — not wreckage, but consequence.*

---

## Setting

The Colossus passed through this region long ago. Its mass disturbed the dust and gas here — compressed it, shifted it, changed the conditions. The Colossus didn't intend this. It simply moved, and matter responded to something that massive moving through it.

New stars are forming in its wake. Not many. A few. Young, bright, electromagnetically loud. The space is alive in a way Stage 1 wasn't.

The player enters a place that has light in it, and warmth, and noise. For the first time, they are not alone in the void.

---

## Mode

Free-roaming. Full player thrust authority. The same controls as Stage 1. What changes is the space.

---

## Threat

Oxygen. Same rules, same ghost charge system as Stage 1. Ghost pools are per-stage — Stage 1 ghosts remain in Stage 1. Stage 2 accumulates its own history of deaths across visits.

The ghost clusters here are mislocated: near the young stars, off the signal path. Usable but requiring a detour. The player must decide whether a ghost is worth the deviation.

See `mechanics/oxygen.md`.

---

## Seeking

Stage 1 taught the seeking loop in silence. Stage 2 is the first test of that skill against interference.

Young stars broadcast. Whatever they emit — electromagnetic radiation, charged particles, the noise of something still igniting — it competes with the Colossus's signal. The signal is still there. It hasn't changed. But now there are other things in the frequency, and the player must learn to hold the thread.

What makes the Colossus's signal distinct is not explained. It is discovered. The player loses the signal near a young star. They stop. They rotate. They find it again on the far side of the noise. Over time they develop an ear for it — something in the signal's irregularity, its organic quality, that the stellar noise doesn't share. The stellar noise is loud but regular. The Colossus's signal is quieter but alive.

See `mechanics/seeking_mechanic.md` — **Noise Interference**.

---

## The Pioneer

At the far edge of the young star field — the point where the EM noise finally dies away and the signal becomes clean again. Not near the young stars. Past them, in the quiet on the other side.

This Pioneer made it through the noise. Every wrecked ship in the stage died turned toward the light. This one kept the signal and followed it to the boundary. It stopped here, still reaching, oriented away from the stars and toward whatever comes next.

Older than Stage 1's Pioneer — how much older is not stated. The player reads it through what the environment has done to it, which is very little. The void at the edge of the star field is quiet. Things drift here without being disturbed. The Pioneer has been drifting at the boundary for a long time.

The moment of finding the Pioneer and the moment the noise finally resolves are the same moment. The player breaks through the interference and there it is — clear signal, clear figure, still pointing forward.

---

## Ghost Behavior

Stage 2 ghosts are seekers who got distracted by the light.

They had the signal. They were moving. Then the young stars appeared — warm, bright, the first color that wasn't cold blue — and something in them turned toward the light instead of the signal. Not a mistake exactly. The stars are beautiful. The warmth is real. But the signal is elsewhere, and they followed the wrong thing until they ran out of oxygen.

Their wrecks are near the young stars, not along the signal path. Oriented toward the light, not the source. The player reads this pattern across visits — all those ships pointing toward the bright things — and understands something about the danger of beauty in a place that's trying to kill you.

Player death ghosts render as wrecked ships — the same form as everywhere else. What makes Stage 2 ghosts readable is position and orientation: if a ghost is near a young star, facing it, the player knows what happened.

See `mechanics/ghosts.md`.

---

## Return: Memory Fragment

*Trigger: warm lights in cold dark, the specific look of home from outside*

From outside, at night, looking at the two houses both lit. The front house and the back house, two separate lights in the dark. Standing outside before going in — that specific pause before the threshold, when you could see where you were going and it was warm and you were not yet there. That pause had its own quality. You always noticed it.

See `mechanics/memories.md` for the crystallization moment and accumulation mechanics.

---

## Atmosphere

**Visual:**

For the first time, the space has points of warm light. Young stars at various distances — the nearest large enough to navigate around, the farthest just bright points. Dust clouds between them, catching the light on their edges. The cold of Stage 1 is still dominant, but there are warm notes in it now.

Color temperature: still predominantly cold, but with warm interruptions. The young stars introduce the warm end of the palette that will eventually saturate the sky near the Colossus. The player does not know this yet.

The dust clouds have visible depth — the player flies through or around them, density shifting. Not a debris field. Not wreckage. Something growing.

**Audio:**

Stage 1 audio: the signal, and silence.

Stage 2 audio: the signal, and noise. The young stars contribute a low electromagnetic presence — not music, not mechanical, something between static and hum. Felt more than analyzed. When the player is near a young star the noise rises and the signal becomes harder to isolate. When they move away from the stars, the noise drops and the signal clarifies.

The player learns to use the noise as spatial information — loud here means near a young star, near a young star means signal interference, move away from the noise to hear clearly.

---

## Production Layout

Concrete geometry and placement spec. All measurements in world units. Player ship: ~2 units wide. Free-roam flight speed: ~20–25 units/second.

Space stages carry no terrain geometry. The stage is a volume of navigable space populated by a small number of 3D landmark objects, a background starfield (2D, not world geometry), and the Pioneer somewhere in the far region. Total triangle count for placed geometry is under 150 tris — the stage's visual richness comes from the starfield, color, and signal behavior, not from polygon density.

**Navigable volume:** 3,200 units deep (Z: 0 to −3,200), 3,000 units wide (X: −1,500 to +1,500), 2,000 units tall (Y: −1,000 to +1,000). These bounds are soft — nothing stops the player from flying past them, but there is nothing of interest beyond them.

**Pioneer position:** Z −2,850, X +380, Y −120. Past all five young stars, at the quiet edge of the field where the EM noise drops away.

---

### Starfield Layers (2D, zero polygon cost)

*Layer 1 — Background carpet:* Screen-space stars, orientation-responsive, never closer regardless of player position. 200–300 point sprites, color-varied (cold white, blue-white, occasional warm point). This is the dominant visual — dense, overwhelming.

*Layer 2 — Nebula color washes:* 5 large translucent quads at extreme distances (1,500–2,500 units), not collidable. Each is a single rectangle (2 tris). These give the stage its warm quality — yellow-orange dust catching the light of forming stars. Total: 10 tris.

| Quad | Color | Position | Scale |
|------|-------|----------|-------|
| Nebula A | Warm amber (0.85, 0.60, 0.20) | Z −2,000, X −800, Y +400 | 600 × 400 |
| Nebula B | Deep orange (0.80, 0.45, 0.15) | Z −1,500, X +1,000, Y −200 | 500 × 350 |
| Nebula C | Cool yellow (0.90, 0.75, 0.30) | Z −2,500, X 0, Y +600 | 800 × 500 |
| Nebula D | Faint gold (0.70, 0.60, 0.25) | Z −800, X −1,200, Y 0 | 400 × 300 |
| Nebula E | Rust-orange (0.75, 0.40, 0.20) | Z −3,000, X +500, Y −400 | 700 × 600 |

---

### Young Star Objects (3D landmarks)

Five proto-stars scattered through the volume. Each is a core geometry object plus a billboard halo quad. The halo faces the camera always. The core is a small bright sphere (~8 tris). Together: ~12 tris per star.

Stars have a **noise radius** — within this distance the Colossus audio signal degrades. The player must navigate around them rather than through them.

| Star | Position | Color | Noise radius | Notes |
|------|----------|-------|-------------|-------|
| Star 1 | Z −480, X −780, Y +120 | Blue-white (0.80, 0.90, 1.00) | 350 units | Smallest, most energetic |
| Star 2 | Z −850, X +1,100, Y −80 | Yellow-white (1.00, 0.95, 0.75) | 400 units | Solar analog |
| Star 3 | Z −1,300, X −60, Y +200 | Bright white (1.00, 1.00, 0.90) | 500 units | Largest, most obstructive — directly in path |
| Star 4 | Z −1,850, X −620, Y −150 | Orange (1.00, 0.70, 0.35) | 400 units | Most luminous, red-shifted |
| Star 5 | Z −2,300, X +820, Y +60 | Warm gold (1.00, 0.82, 0.40) | 300 units | Nearest to Pioneer; warmest palette |

The Pioneer is ~700 units past Star 5, in the quiet region where the star's noise has faded. Star 5's warm gold is the visual cue — the Pioneer is on the other side of the warmest light.

**Total young star geometry:** 5 × 12 tris = 60 tris.

---

### Foreground 3D Stars (parallax landmarks)

Five individual bright stars as actual 3D geometry — close enough that they show real parallax as the player moves. Not proto-stars, just bright individual points in near space. Each is a small cross or point geometry, ~2 tris.

| Star | Position | Color |
|------|----------|-------|
| Foreground A | Z −280, X +520, Y +340 | Blue-white |
| Foreground B | Z −180, X −660, Y −210 | Cool white |
| Foreground C | Z −440, X +880, Y −90 | Warm yellow |
| Foreground D | Z −350, X −380, Y +450 | Pure white |
| Foreground E | Z −500, X +200, Y −380 | Blue-white |

These are the most immediate 3D objects in the stage. Their parallax drift as the player moves is what separates this from a painted backdrop — the sense of actually occupying space.

**Total foreground star geometry:** 5 × 2 tris = 10 tris.

---

### Pioneer

Position: Z −2,850, X +380, Y −120. Drifting slowly (no velocity). In the quiet past Star 5. Draw distance: 400 units.

The approach: the player breaks through Star 5's noise zone. The signal clarifies. Seconds later the Pioneer resolves out of the starfield through the windshield — it's been there, it was just another point of light until they were close enough to see it.

Older than Stage 1's Pioneer, but the void has been kind — nothing to corrode it, nothing to disturb it. It has drifted slightly from wherever it stopped originally, pushed by the residual stellar wind off Star 5. The direction it faces is still correct.

---

### Ghost Ship Positioning

Ghost ships accumulate near young stars — this is where players die, chasing warmth and light instead of signal. A well-visited stage will show a visible cluster of wrecks near Star 3 (the obstructive central star) and smaller clusters near Stars 1 and 2 (early distractions).

The Pioneer's approach corridor (past Star 5) will be sparse. Few players have made it this far without running out of oxygen first.

Draw distance: 250 units (longer than in planet stages — open space, no occlusion, ghosts should be findable by sight more easily).

---

### Full Object Count

| Category | Count | Tris |
|----------|-------|------|
| Nebula quads | 5 | 10 |
| Young star objects (core + halo) | 5 | 60 |
| Foreground parallax stars | 5 | 10 |
| Pioneer | 1 | — |
| **Total placed geometry** | **16** | **~80 tris** |

Fewer than 100 tris of stage geometry for the entire space. The visual complexity is entirely the 2D starfield and signal behavior. This is the right approach — open space should feel empty, not dense.

---

## Open

- Is there a specific landmark or moment that defines the stage end — a formation, a particular star, something that reframes the space the way the Pioneer reframes Stage 1? Or does Stage 2 end simply when the signal leads out of the young star field into clearer space?
- How dense is the star field? A few bright points widely spaced (sparse, navigable) or a crowded nursery (dense, disorienting)?
- Do the dust clouds create spatial dead zones — areas where the signal drops completely — or only noise interference?
