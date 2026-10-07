# Step 06: register effects

## Goal

All one-shot register effects on every channel, including folding an effect into a trigger on the same row. After this step, every golden song in `tests/songs/` is a driver test.

## Scope

In:
- **5:** `NR50=xx`. **8:** `NR51=xx`. **6:** no-op. With a note on the row, 5 and 8 are written after that channel's trigger.
- **9 without a note:**
  - ch1–2: `NRx1=xx`;
  - ch3: wave `y` = `xx & $0F`. If it isn't loaded: [wave load], `NR30=$80`, `NR34` = `$80` | len | period bits 10–8. If it is loaded: nothing;
  - ch4: `NR43` = noise value of the last note | (`xx` ≠ 0 ? `$08` : 0).
- **9 with a note** changes the trigger instead of making extra writes:
  - ch1–2: `NRx1=xx`;
  - ch3: the trigger uses wave `y`;
  - ch4: the LFSR width is `xx` ≠ 0.
- **C with a note** changes the trigger:
  - ch1, 2, 4: `NRx2=xx`;
  - ch3: `NR32` = (`x` & 3) << 5.
- **9 and C are not remembered:** the next trigger takes the instrument's values again.

Out:
- the effects not specified yet (0–4, 7, A, E). The driver ignores them; the reference player rejects them.

## Design notes

- **Effect state in WRAM.** The effect and parameter of the cell being played are kept in `wEffect` and `wParam`, because the trigger code needs `bc`, `de` and `hl`.
- **`wOverride`** is set before the trigger: `OVERRIDE_TIMBRE` for 9 or `OVERRIDE_VOLUME` for C on a row with a note. Each trigger checks it where it reads the instrument value. After an overridden trigger, the effect is complete.
- **Row order:** instrument column, flow effects, trigger (if there is a note), then `ApplyEffect`: 5, 8, and 9 and C without a note.
- **Channel 4's noise value** (the NR43 shift and divider of the last note, without the width bit) is kept in the channel state, so 9 without a note can rewrite `NR43`.

## Acceptance criteria

- **`driver.registers` passes** (existing `registers.gsong`): 5, 8, 9 and C, with and without a note, on every channel, and 6.
- **`driver.effects` passes.** The new song (`tests/songs/effects.gsong`, 600 frames) covers the cases `registers` doesn't:
  - 5 and 8 after a trigger on the same row, on several channels;
  - 9 with a note on the wave channel when that wave is already loaded;
  - 9 on channel 4 before any note (noise value 0);
  - C with a note on channel 3 and channel 4;
  - the instrument's values coming back on the next trigger after 9 and C.
- All earlier driver tests still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
