# Step 04: channel 4 *(outline)*

## Goal

The noise channel plays notes through the generated `NoiseNr43` table. With it, all four channels are complete.

## Scope

In:
- Noise instruments.
- The channel 4 trigger: `NR41 NR42 NR43 NR44`, with NR43 = table value | width bit from the instrument.
- `C` without a note: `NR42=xx`, `NR44=$80 | len`.

Out: effect 9 (step 06).

## Acceptance criteria

- **New golden song `noise.gsong`:** both LFSR widths, the lowest and highest notes, and length enable.
- **Existing songs now supported:** `driver.noise`, `driver.minimal` and `driver.instruments` pass.
- All earlier driver tests still pass.
