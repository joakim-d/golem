# Step 01: channel 1 plays a scale

## Goal

The smallest vertical slice: one square channel plays notes with note on and note off, and its trace matches the reference player's frame by frame.

## Scope

In:
- **Song reading:** the header, the order count, channel 1's order table, the pulse instruments, and the pattern cells.
- **Timing:** ticks per row (0 = 256), 64 rows per pattern, orders in sequence, and the loop back to order 0.
- **Instrument column:** sets the current instrument, which is 1 at start.
- **Note:** a full channel 1 trigger: `NR10 NR11 NR12 NR13 NR14`, with the period taken from the generated `NotePeriods` table.
- **Note off:** effect `C` on a row without a note writes `NR12=xx`, then retriggers `NR14`.

Out (later steps):
- channels 2–4;
- all other effects;
- `C` on a row that also has a note (folded into the trigger, step 06).

## Design notes

- **Tick wrap:** the tick counter is compared with the raw ticks-per-row byte. Because it wraps from 255 to 0, a raw 0 gives a 256-tick row with no special case.
- **NR14** = `$80` | (instrument byte 0 bit 7 moved to bit 6) | period bits 10–8. The retrigger after `C` uses the current instrument and the period of the last note.

## Acceptance criteria

**`driver.scale` passes.** The song (`tests/songs/scale.gsong`, 800 frames) has two orders:
- Order 0 plays C-4 to C-5 upwards with the default instrument 1.
- Order 1 plays them downwards with instrument 2 (length enabled).
- Every note is followed two rows later by `C00`.
- The 800 frames include the loop back to order 0, which keeps instrument 2.

`driver.silence` still passes.

## Verification

```sh
cmake --workflow --preset linux-debug
build/linux-debug/tools/golem-run build/linux-debug/driver/scale.gb --frames 26
```

Frame 1 holds the C-4 trigger (`NR13=0B`, `NR14=86`). Frame 13 holds the note off (`NR12=00`, `NR14=86`).
