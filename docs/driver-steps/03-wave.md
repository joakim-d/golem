# Step 03: channel 3

## Goal

The wave channel plays notes, loading waves into wave RAM only when they change.

## Scope

In:
- **Wave instruments:** byte 0 → `NR31`; byte 1 holds the length enable (bit 7), the `NR32` volume code (bits 6–5) and the wave index (bits 3–0).
- **Channel 3 trigger:**
  - If the instrument's wave differs from the one in wave RAM: `NR30=$00`, then the 16 bytes of the wave to `$FF30`–`$FF3F`.
  - Then `NR30=$80 NR31 NR32 NR33 NR34`, with `NR34` = `$80` | len | period bits 10–8.
- **The loaded wave** is remembered, with none loaded at start, so the first trigger always loads.
- **`C` without a note** on channel 3: `NR32` = (`x` & 3) << 5, with no retrigger.

Out:
- effect 9 (wave change) and `C` on a row with a note (step 06);
- channel 4 (step 04).

## Design notes

- **`wLoadedWave`:** `$FF` means no wave loaded (wave indexes are 0–15).
- **Dispatch:** the channel loop calls the wave code for channel 3 (`wChannel` = 2) and the pulse code for channels 1–2. The period of the last note stays in the per-channel state, like the pulse channels.

## Acceptance criteria

**`driver.wave` passes.** The song (`tests/songs/wave.gsong`, 600 frames) covers:
- the first trigger loading a wave;
- the same wave instrument twice in a row (no reload);
- a different instrument using the same wave (no reload);
- a switch to another wave (reload);
- all four `NR32` volume codes;
- length enable;
- `C` without a note;
- two orders and the loop. The loop must not reload the wave still in RAM.

`driver.silence`, `driver.scale` and `driver.pulses` still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
