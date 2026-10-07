# Step 10: volume slide (A)

## Goal

The first continuous effect: A slides the volume of channels 1, 2 and 4 on every non-row tick of its row, as specified in the contract ("Effects", "Volume") and implemented in the reference player first.

## Scope

In:
- **A per-channel volume (0–15)**, kept up to date by:
  - triggers (the high nibble of the `NRx2` they write, including a folded C);
  - C without a note;
  - E (which sets it to 0).
- **A `xy`:** on each non-row tick, the volume goes up by `x` if `x` ≠ 0, else down by `y`, clamped to 0–15. When it changes, the driver writes `NRx2` = volume << 4 (envelope pace 0), then retriggers `NRx4`. When it doesn't, nothing is written.
- **Channel 3** ignores A.
- **Order:** due timed effects and slide steps are written in channel order.
- **WRAM budget:** raised to 128 bytes (see [07-budgets.md](07-budgets.md)).

Out: the other continuous effects (0–4).

## Design notes

- **`wSlides`:** one byte per channel, next to `wCutTicks` and `wDelayTicks`, cleared with them on every row tick. 0 means "no slide", which is also what A00 does. `wTimedPending` covers slides, so a tick with nothing pending stays cheap.
- **`wVolumes`:** one byte per channel. It is set by `StoreVolume` wherever `NRx2` / `NR42` is written for a trigger or a C.
- **A slide step** reuses the "C without a note" code (`SetVolume`) with `wParam` = volume << 4. Those are exactly the writes a slide makes.

## Acceptance criteria

**`driver.slide` passes.** The song (`tests/songs/slide.gsong`, 600 frames) covers:
- slides up and down;
- clamping at 15 and at 0, and `A00`;
- starting volumes from the instrument, a folded C, C without a note, and a cut;
- the noise channel, and the wave channel (ignored);
- a slide step and a cut on the same tick;
- a long row at another tempo.

All earlier driver tests and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
