# Step 05: flow control

## Goal

Tempo changes, position jumps and pattern breaks, with the rules from the "Flow control" section of the contract.

## Scope

In:
- **F `xx`:** ticks per row = `xx` (0 = 256), starting from the next row. The current row keeps the length it started with, and the tempo persists across the loop.
- **B `xx`:** after this row, continue at order `xx`, row 0.
- **D `xx`:** after this row, continue at the next order, row `xx`.
- **Combining B and D:**
  - B and D on the same row (any channels) give order B, row D.
  - The highest channel wins between several Bs or several Ds.
  - Out of range (order ≥ order count, row ≥ 64) goes to 0, and D on the last order wraps to order 0.
- **Flow effects apply whether or not the row has a note.** They make no APU writes, so their place relative to the trigger doesn't matter.

Out:
- effects 5, 6, 8, 9, and `C` on a row with a note (step 06).

## Design notes

- **Row length** is copied from the tempo when a row starts (`wRowLength`), so an F on that row only changes the next one.
- **Pending flow:**
  - B and D set flags in `wFlow` and store their targets in `wJumpOrder` and `wBreakRow`.
  - Channels are processed in order, so a later channel overwrites the target: the highest channel wins.
  - At the end of the row, a pending flow replaces the normal "next row" step, then is cleared.

## Acceptance criteria

- **`driver.flow` passes** (existing `flow.gsong`: F with a delayed effect, D, B, the loop keeping the tempo).
- **`driver.jumps` passes.** The new song (`tests/songs/jumps.gsong`, 600 frames, 3 orders) covers:
  - B on a row with a note;
  - B and D on the same row, from different channels;
  - the highest channel winning, for B (channels 1 and 3) and for D (channels 2 and 4);
  - an out-of-range B (`B09`) and D (`D50`);
  - D on the last order wrapping to order 0;
  - `F00` (a 256-frame row), and F on rows with notes.
- All earlier driver tests still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
