# Step 11: optimize the row tick

## Goal

Make room in the cycle budget before the per-tick pitch effects (0–4), without changing any APU write. After step 10, the worst case is 11792 of 14000 cycles, and an empty song's row tick costs 3512 cycles: about 850 per channel to play nothing.

## Scope

In, measured one change at a time:
1. **Cache the song header's pointers** in WRAM at `GolemInit`: the four order tables, the three instrument tables and the waves. They replace the `SongHeaderWord` lookups (about 120 cycles each), which happen for every channel on every row and for every instrument lookup.
2. **Cache each channel's pattern pointer.** It is loaded from the order table only when the order changes (start, next order, B/D). Each row then only adds row × 3.
3. **Empty cells take a fast path.** A cell of three zero bytes (no note, no instrument, effect `0` with `$00`) does nothing, so it skips the instrument, flow, override and effect code.
4. **Look up the instrument once per trigger.** `PulseNrx4` and its wave and noise equivalents get the instrument from their caller instead of looking it up again.

Out:
- changing the budget (14000 cycles stays, for 0–4);
- any change to behaviour or to the reference player.

## Acceptance criteria

- **No behaviour change:** every `driver.*` trace test passes unchanged; no golden trace is regenerated.
- **Measured and recorded:** the `GolemPlay` worst case and the empty-song averages fall, and the new values go in [07-budgets.md](07-budgets.md) next to the old ones.
- **Within budget:** ROM and WRAM grow only by the caches.

## Results

Each change was measured on its own over the 15 driver songs, and every trace stayed identical:

| Change | Worst case | Mean of averages |
|---|---|---|
| Before (step 10) | 11792 | 1083.9 |
| 1. Header pointers in WRAM | 11408 | 1051.8 |
| 2. Pattern pointers per order | 10992 | 960.4 |
| 3. Empty-cell fast path | 11072 | 674.2 |
| 4. One instrument lookup per trigger | 9688 | 659.9 |

- **Change 3 raised the worst case slightly:** the worst-case row has notes on every channel, so each cell pays the extra check and none skips.
- **A regression the traces caught:** the first version of change 3 overwrote the register that held the cell's effect, and `driver.cut` failed at frame 1 until it was fixed.
- **The song pointer (`wSong`) is gone:** once the header is cached, nothing reads it.

## Verification

```sh
cmake --workflow --preset linux-debug
for s in silence minimal slide cut; do
  build/linux-debug/tools/golem-run build/linux-debug/driver/$s.gb --expect tests/songs/$s.trace --cycles
done
```
