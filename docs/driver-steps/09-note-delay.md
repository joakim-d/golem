# Step 09: note delay (7)

## Goal

The second timed effect: 7 `xx` moves the row's trigger to tick `xx`, as specified in the contract ("Effects", "Note delay") and implemented in the reference player first.

## Scope

In:
- **7 00:** an ordinary trigger on the row tick.
- **7 `xx` ≥ 1 with a note:**
  - no trigger on the row tick;
  - the instrument column still applies then;
  - the trigger happens at tick `xx`, or never if `xx` ≥ the length the row started with.
- **7 without a note:** nothing.
- **Due delayed triggers and cuts** are written in channel order on non-row ticks.

Out: the continuous effects (0–4, A).

## Design notes

- **`wDelayTicks` and `wDelayNotes`:** one byte per channel each, next to `wCutTicks`. 0 means "nothing pending", for the same reason as `NO_CUT`.
- **`wTimedPending`:** the step 08 flag `wCutPending`, renamed. It now covers both effects, so `PlayTimed` (formerly `PlayCuts`) still costs almost nothing on ticks without a pending effect.
- **Delayed triggers** call the existing `Trigger` with `wOverride` cleared. The row's 9 or C can't apply, since the cell's effect is 7.

## Acceptance criteria

**`driver.delay` passes.** The song (`tests/songs/delay.gsong`, 600 frames) covers:
- 700;
- delays on pulse, wave (with its wave load) and noise;
- an instrument change on a delayed row;
- a dropped delay;
- 7 without a note;
- a delayed trigger and a cut on the same tick;
- a later retrigger using the delayed note's period.

All earlier driver tests and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
