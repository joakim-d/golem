# Step 07: budgets *(outline)*

## Goal

Measure the cost of the driver and make the tests fail when it grows past agreed limits.

## Scope

In:
- **Cycles per `GolemPlay` call:** the maximum and the average over each golden song. `golem-run` measures them by counting emulated cycles between the marker and the return, or with an extra marker after `GolemPlay`.
- **ROM size** of the driver section and **WRAM size** of its state, read from the `rgblink` map file.
- **Limits** chosen in this step from the measured values plus headroom, and checked by ctest.

## Acceptance criteria

- **Each golden song's driver test also reports cycles per frame,** and fails above the limit.
- **A size test fails** when the driver's ROM or WRAM grows past its limit.
- The limits and how they were chosen are written in this file.
