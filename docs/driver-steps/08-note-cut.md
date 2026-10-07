# Step 08: note cut (E)

## Goal

The first timed effect: E `xx` silences the channel at tick `xx` of its row, with the same writes as C00. It is specified in the contract ("Effects", "Note cut") and implemented in the reference player first.

## Scope

In:
- **E00:** the cut is written right after the row's trigger, or on its own when the row has no note, as the channel's effect.
- **E `xx` ≥ 1:** the cut is written at tick `xx` of the row, on a non-row tick, in channel order. Nothing happens if `xx` ≥ the length the row started with.
- **The cut's writes:** C00's: ch1, 2, 4: `NRx2=$00`, then `NRx4` retrigger; ch3: `NR32=$00`.

Out: the other timed effect, 7 (note delay).

## Design notes

- **`wCutTicks`:** one byte per channel holds the tick of a pending cut, or `NO_CUT` (0: tick 0 never reaches the check, since E00 cuts at once, while every tick 1–255 can be real in a 256-tick row). It is cleared on every row tick before the row is played.
- **Non-row ticks:** `GolemPlay` checks `wCutTicks` against `wTick` before advancing the tick. `wCutPending` skips that check when the row set no cut: without it, the loop over the four channels on every tick doubled the average cycles per call (627 → 1284 for an empty song).
- **Writes:** a cut calls the channel's existing "C without a note" code with a parameter of 0.

## Acceptance criteria

**`driver.cut` passes.** The song (`tests/songs/cut.gsong`, 600 frames) covers:
- cuts at tick 0 and at later ticks, with and without a note;
- a cut at the row's length (no cut);
- cuts on every channel;
- two channels cutting on the same tick;
- F on the same row as a cut;
- the loop at a shorter tempo, where earlier cuts fall past the row.

All earlier driver tests and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
