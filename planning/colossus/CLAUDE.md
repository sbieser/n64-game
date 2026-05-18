# Planning — Colossus

Design documentation for *The Colossus*. Follow these rules when reading or writing any file in this folder.

## Folder Structure

```
mechanics/   — one file per game mechanic
stages/      — one file per stage
research/    — reference material, competitive analysis
```

Root files (`design_game_structure.md`, `colossus.md`) cover the overall arc, themes, and the nature of the Colossus entity.

## Rule: Stage docs describe a place. Mechanic docs describe a mechanic.

A stage document covers: the setting, the narrative, what makes this stage distinct, and how existing mechanics express themselves here. It does not explain how any mechanic works.

A mechanic document covers: how the mechanic works, all its states and rules, its design rationale. It does not describe any specific stage.

When a stage uses a mechanic, it references the mechanic file by name. It does not restate the mechanic.

## Rule: New mechanic = new file in mechanics/

When the game introduces something worth naming as a mechanic — oxygen, ghosts, seeking, controls — it gets its own file. Even if it turns out to be stage-specific, it lives in mechanics/ and the stage doc references it.

## Rule: Stages may introduce, amplify, or simply use mechanics

A stage does not need to introduce anything new. It may apply existing mechanics in a harder or stranger way. The stage doc notes which mechanics are active and what is specific to this stage's expression of them. The mechanic doc covers the full possibility space.
