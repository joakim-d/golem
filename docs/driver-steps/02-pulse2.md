# Step 02: channel 2

## Goal

Channel 2 plays alongside channel 1. The driver moves from channel 1 globals to per-channel state and a loop over channels, the structure that steps 03–06 build on.

## Scope

In:
- **Channel processing in order** 1 → 4 on the row tick; for now only the pulse channels (1, 2) do anything.
- **Per-channel state** in WRAM: current instrument (1 at start) and the period of the last note.
- **Channel 2 trigger:** `NR21 NR22 NR23 NR24`. Pulse instrument byte 0 contributes only its length-enable bit; `NR10` is written for channel 1 only.
- **Instrument column and note off** (`C` without a note) on both pulse channels.

Out:
- channels 3–4 (steps 03–04);
- every effect other than `C` without a note;
- `C` on a row with a note (step 06).

## Design notes

- **One code path for both pulse channels.** Registers are addressed with `ldh [c], a`, where `c` is the low byte of NRx1 (`$11` for channel 1, `$16` for channel 2; the stride is 5).
- **Channel state:** `wChannels` holds 4 bytes per channel, indexed by `wChannel`, the channel being played (0-based): instrument, then period (2 bytes, little-endian).

## Acceptance criteria

**`driver.pulses` passes.** The song (`tests/songs/pulses.gsong`, 600 frames) covers:
- both channels triggering on the same row and on different rows;
- instrument switches on both channels, including an instrument column without a note;
- an instrument with length enabled on each channel (NRx4 bit 6), and one with a sweep byte, which is written to `NR10` on channel 1 and ignored on channel 2;
- note offs on both channels;
- two orders and the loop.

`driver.silence` and `driver.scale` still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
