# Step 02: channel 2 *(outline)*

## Goal

Channel 2 plays alongside channel 1. This is where the driver starts processing channels in order, 1 → 4.

## Scope

In:
- Channel 2 triggers: `NR21 NR22 NR23 NR24` (pulse instrument; byte 0 contributes only its length-enable bit).
- Instrument column and note off (`C` without a note) on channel 2.
- Per-channel state instead of channel 1 globals.

Out: channels 3–4, and every effect other than `C` without a note.

## Acceptance criteria

- **New golden song `pulses.gsong`:**
  - both pulse channels play on the same rows and on different rows;
  - instruments switch on both channels;
  - one instrument has length enabled, so the NRx4 bit 6 matters;
  - note off on both channels.
- `driver.pulses` passes, and `driver.silence` and `driver.scale` still pass.
