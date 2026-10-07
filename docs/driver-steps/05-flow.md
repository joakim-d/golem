# Step 05: flow control *(outline)*

## Goal

Tempo changes, position jumps and pattern breaks, with the rules from the "Flow control" section of the contract.

## Scope

In:
- **F:** new ticks per row from the next row; 0 = 256; the tempo persists across the loop.
- **B, D:** resolved after all four channels. B and D on the same row combine; the highest channel wins; out-of-range values go to 0; D on the last order wraps.

## Acceptance criteria

- **`driver.flow` passes** (existing `flow.gsong`).
- **A new golden song covers the remaining flow rules:** B and D on the same row, the highest channel winning, out-of-range values, and `F00`. Its driver test passes.
- All earlier driver tests still pass.
