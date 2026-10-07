# Step 06: register effects *(outline)*

## Goal

All one-shot register effects on every channel, including the cases where an effect is folded into a trigger on the same row.

## Scope

In:
- **5:** `NR50`. **8:** `NR51`. **6:** no-op.
- **9 per channel:**
  - ch1–2: NRx1;
  - ch3: wave reload and retrigger, nothing if the wave is already loaded;
  - ch4: width, written to NR43.
- **C on every channel.**
- **Folding:** 9 and C on a row with a note change the trigger's values instead of making extra writes. 5 and 8 are written after that channel's trigger.

## Acceptance criteria

- **`driver.registers` passes** (existing `registers.gsong`).
- All earlier driver tests still pass. By now every existing golden song in `tests/songs/` is a driver test.
