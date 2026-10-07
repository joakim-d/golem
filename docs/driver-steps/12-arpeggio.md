# Step 12: arpeggio (0)

## Goal

The first pitch effect: 0 `xy` cycles the channel's last note through +0, +`x`, +`y` on the non-row ticks of its row, then the next row tick puts the note's pitch back. As specified in the contract ("Effects", "Pitch") and implemented in the reference player first.

## Scope

In:
- **Arpeggio steps:** on non-row tick `t`, the last note + (0, `x`, `y`)[`t` % 3], clamped to B-7. When the note changes, `NRx3` and `NRx4` are written without the trigger bit (with the length bit), on channels 1–3. Nothing before the channel's first note; nothing on channel 4.
- **Restore:** on every row tick, before a channel's cell, if its pitch differs from its last note, that note's period is written back, unless the cell triggers on this tick.
- **Order:** due timed effects, slide steps and arpeggio steps are written in channel order.

Out: portamento and vibrato (1–4).

## Design notes

- **Pitch as a note:** the driver tracks the sounding pitch as a note (`wPitchNotes`) next to the last note (`wNotes`). Periods only depend on notes and strictly increase with them, so comparing notes is enough, and cheaper than comparing 16-bit periods.
- **`wArpeggios`:** one byte per channel, cleared on each row tick with the cut, delay and slide tables. A parameter of `$00` means no arpeggio; an empty cell is the same thing.
- **`wPhase`** keeps `wTick % 3` up to date as the tick advances, so a step needs no division.
- **The restore check comes before the empty-cell fast path,** so it also runs on rows whose cell is empty.
- **`wPitchMoved`:** set by an arpeggio step that moves a pitch, cleared at the end of each row tick (by then every pitch is back on its note). The restore checks only run while it is set. Without it, the checks on every row tick of every song raised the mean of the songs' averages from 660 to 917 cycles and the worst case to 11040.

## Results

All 16 driver traces pass.

| Measure | Step 11 | Step 12 |
|---|---|---|
| `GolemPlay` worst case | 9688 cycles | 10424 cycles (74% of the budget) |
| Mean of the songs' averages | 660 cycles | 780 cycles |
| Empty song's row tick | 1624 cycles | 1940 cycles |
| ROM / WRAM | 1446 / 75 bytes | 1711 / 89 bytes |

The remaining cost is the arpeggio's bookkeeping, paid by every song:
- triggers record their note and pitch;
- every tick advances `wPhase`;
- each row tick checks `wPitchMoved` once per channel.

## Acceptance criteria

**`driver.arpeggio` passes.** The song (`tests/songs/arpeggio.gsong`, 600 frames) covers:
- the full cycle;
- `y` = 0, and clamping at B-7;
- the length bit;
- an arpeggio before any note, and on the last note;
- the restore before C, before a delayed note, and on a channel whose next row is empty;
- no restore when the next row triggers;
- the wave channel, and the noise channel (ignored);
- two channels stepping on the same tick.

All earlier driver tests and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
