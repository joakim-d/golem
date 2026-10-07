# Step 15: vibrato (4)

## Goal

The last effect: 4 `xy` vibrates the pitch around the channel's period as a centred square wave, +`y` for `x` ticks then −`y` for `x` ticks, restarting every row. As specified in the contract ("Effects", "Period and pitch") and implemented in the reference player first. After this step, every effect of the song format is implemented in both the reference player and the driver.

## Scope

In:
- **Vibrato steps:** on non-row tick `t`, the period + `y` for `t` = 1 to `x`, − `y` for the next `x` ticks, and so on (`x` = 0 counts as 1), clamped to the note table, on channels 1–3.
  - The steps move the pitch only. The next row tick restores the period, as after an arpeggio.
  - When the pitch changes, `NRx3` and `NRx4` are written without the trigger bit.
- **Nothing** before the channel's first note, and nothing on channel 4.
- **The reference player drops its "unsupported effect" path:** every effect code 0–F is now specified.

## Design notes

- **`wVibratos` and `wVibratoCounts`:** the row's parameter and a counter per channel, both cleared on each row tick with the other per-row effect tables. The counter holds the current phase (bit 7: −`y`) and the ticks left in it (bits 0–6). 0 means "first step of the row", so the clearing restarts the vibrato with no extra code and no division.
- **Shared clamping:** `AddPeriodClamped` and `SubPeriodClamped` clamp to the note table for both `PortaStep` and `VibratoStep`. They replace the inline code `PortaStep` had.

## Results

All 19 driver traces pass. `driver.portamento` is unchanged by the move to the shared clamp routines.

| Measure | Step 14 | Step 15 |
|---|---|---|
| `GolemPlay` worst case | 11732 cycles | 12004 cycles (86% of the budget) |
| Mean of the songs' averages | 871 cycles | 931 cycles |
| ROM / WRAM | 2098 / 113 bytes | 2240 / 121 bytes |

## Acceptance criteria

**`driver.vibrato` passes.** The song (`tests/songs/vibrato.gsong`, 600 frames) covers:
- speeds 1, 2 and 0, and depth 0;
- clamping at B-7 and C-2;
- a vibrato before any note, and on a slid period;
- the restore before C;
- the wave channel, and the noise channel (ignored);
- two channels on the same ticks.

All earlier driver tests (`driver.portamento` included, since its clamping moves into the shared routines) and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
