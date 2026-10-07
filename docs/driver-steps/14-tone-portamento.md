# Step 14: tone portamento (3)

## Goal

3 `xx` glides the channel's period toward a target note at `xx` per non-row tick, without passing it. As specified in the contract ("Effects", "Tone portamento target") and implemented in the reference player first.

## Scope

In:
- **A note with 3,** on a channel that already plays a note: no trigger. The note becomes the channel's note, and its period the target. On the channel's first note: an ordinary trigger.
- **Steps:** on channels 1–3, the period moves toward the target, without passing it. When it changes, `NRx3` and `NRx4` are written without the trigger bit. The new period stays the channel's period.
- **3 on later rows without a note** keeps sliding toward the target. `300`, or no target, does nothing. A new trigger clears the target.
- **Restore:** the row-tick restore now treats a note with 3 as not triggering.
- **Channel 4:** 3 is ignored; the note simply triggers.
- **ROM budget:** raised to 3072 bytes (see [07-budgets.md](07-budgets.md)).

Out: vibrato (4).

## Design notes

- **`wTargets`:** one 16-bit period per channel, kept from row to row; 0 means "no target", since note periods are at least 44. Triggers clear it in `SetNotePeriod`.
- **`wTonePortas`:** the row's step, cleared with the other per-row effect tables.
- **The step size** is passed to `TonePortaStep` through `wParam`, which is free on non-row ticks, so `bc` and `de` can hold the target and the period.

## Results

All 18 driver traces pass.

| Measure | Step 13 | Step 14 |
|---|---|---|
| `GolemPlay` worst case | 11012 cycles | 11732 cycles (84% of the budget) |
| Mean of the songs' averages | 825 cycles | 871 cycles |
| ROM / WRAM | 1892 / 101 bytes | 2098 / 113 bytes |

**WRAM is at 113 of 128 bytes.** Vibrato needs per-channel state, which leaves very little room.

## Acceptance criteria

**`driver.tone` passes.** The song (`tests/songs/tone.gsong`, 600 frames) covers:
- the first note triggering;
- slides up and down that stop on their target;
- a slow glide over several rows;
- an arpeggio based on the target note;
- `300`, 3 without a target, and a new note clearing the target;
- the wave channel, and the noise channel (ordinary triggers);
- two channels sliding on the same tick.

All earlier driver tests and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
