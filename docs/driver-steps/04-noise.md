# Step 04: channel 4

## Goal

The noise channel plays notes through the generated `NoiseNr43` table. With it, all four channels are complete, and the first existing golden songs become driver tests.

## Scope

In:
- **Noise instruments:**
  - byte 0 holds the LFSR width (bit 7), the length enable (bit 6) and the length timer (bits 5–0);
  - byte 1 → `NR42`.
- **Channel 4 trigger:** `NR41` = byte 0 & `$3F`, `NR42` = byte 1, `NR43` = `NoiseNr43[note]` | (`$08` if 7-bit LFSR), `NR44` = `$80` | (byte 0 & `$40`).
- **`C` without a note** on channel 4: `NR42=xx`, then `NR44` = `$80` | len.
- **The channel loop covers all four channels.**

Out:
- effect 9 (LFSR width, which needs the noise value of the last note) and `C` on a row with a note (step 06).

## Design notes

- **Length bit:** noise instrument byte 0 already has the length-enable bit at bit 6, its place in `NR44`, so no shift is needed (unlike the pulse and wave instruments).
- **NR43 table:** `NoiseNr43` comes from `golem-tables`, so the driver's values are the reference player's `noise_nr43()`.

## Acceptance criteria

- **`driver.noise` passes.** The song (`tests/songs/noise.gsong`, 600 frames) covers:
  - both LFSR widths;
  - the lowest and highest notes;
  - instruments with and without length enable;
  - instrument switches, including an instrument kept across rows;
  - `C` without a note;
  - two orders and the loop.
- **Existing golden songs now run as driver tests:** `driver.minimal` (one note on every channel, 600 frames) and `driver.instruments` (every instrument type) pass.
- `driver.silence`, `driver.scale`, `driver.pulses` and `driver.wave` still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
