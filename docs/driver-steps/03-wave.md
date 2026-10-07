# Step 03: channel 3 *(outline)*

## Goal

The wave channel plays notes and loads waves into wave RAM only when they change.

## Scope

In:
- Wave instruments.
- The channel 3 trigger: `[NR30=$00, $FF30..$FF3F]` when the wave changes, then `NR30=$80 NR31 NR32 NR33 NR34`.
- Remembering the loaded wave (none at start).
- `C` without a note: `NR32=(x & 3) << 5`.

Out: effect 9 (step 06).

## Acceptance criteria

- **New golden song `wave.gsong`:**
  - the same wave twice in a row (no reload);
  - a switch to another wave instrument (reload);
  - the three volume codes;
  - length enable.
- `driver.wave` passes, and all earlier driver tests still pass.
