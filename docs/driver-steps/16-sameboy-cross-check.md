# Step 16: cross-check in SameBoy

## Goal

Confirm that what the tests measure in Peanut-GB holds in an accurate emulator: the driver's APU writes, and the cycles each call takes. Every driver test then checks the traces and the cycles in both emulators.

## Scope

In:
- **SameBoy v1.0.3** (Expat license), downloaded with a checksum like Peanut-GB.
  - Only its emulator core is built; the debugger, cheats and rewind are left out.
  - It is only built with GCC or Clang, because its core relies on GNU C extensions. With MSVC, the tests run in Peanut-GB only.
- **Emulator choice:** `run_rom` takes an emulator (`Emulator::PeanutGb` or `Emulator::SameBoy`), and `available_emulators()` lists the emulators built in.
- **`golem-run --emulator peanut|sameboy|both`:** `both` runs the ROM in each emulator, checks each trace (against the golden trace with `--expect`, or against each other otherwise), and reports any frame whose call takes a different number of cycles.
- **Driver tests:** every `driver.<song>` test uses `--emulator both` when SameBoy is available.

Out:
- sound emulation, which the comparison does not involve;
- other Game Boy models (the test runs a DMG).

## Design notes

- **Boot ROM:** SameBoy starts in its boot ROM. Golem uses a 256-byte stand-in that does only what the test ROM relies on: it turns the LCD on (`LCDC=$91`, as the real DMG boot ROM leaves it, so that VBlank interrupts happen), then unmaps itself (`$FF50`) as execution reaches `$0100`. Without it, the test ROM's `halt` never wakes up.
- **Headless:** rendering is disabled (`GB_set_rendering_disabled`), since SameBoy otherwise needs a pixel buffer.
- **Turbo mode:** SameBoy paces itself to real time by default. The first integration took 13.4 s per song (800 frames at exactly 60 per second), and 176 s for the driver tests. `GB_set_turbo_mode(gb, true, true)` runs as fast as possible: 0.44 s per song. The core is also always built with `-O2`.
- **Timing:** SameBoy runs one instruction per `GB_run`, which returns the time in 8 MHz ticks (2 per CPU cycle on a DMG). The glue keeps the total, so a write is stamped with the time *before* its instruction, the same convention as the Peanut-GB clock. The cycle measurements of the two emulators are therefore directly comparable.

## Prototype result

With a throwaway harness:
- **Traces:** all 18 driver songs give traces identical to their golden traces.
- **Cycles:** `GolemInit`, and `GolemPlay`'s worst case (and the frame it happens in) and average, are identical to Peanut-GB's for every song.

## Result

- **All 19 driver tests** (18 songs and the size check) pass with `--emulator both`: identical traces, and identical cycles for every frame of every song.
- **The `RomRunner` tests** pass in both emulators, including the exact cycle counts of the hand-assembled ROMs (12 and 44).
- **Timing:** the driver and runner tests take about 7 s.

## Acceptance criteria

- **`RomRunner.*` tests run for every available emulator:** frame splitting, cycle measurement on hand-assembled ROMs with known timings, and the timeout. The header-checksum test stays Peanut-GB only, since SameBoy accepts any header.
- **Every `driver.<song>` test passes with `--emulator both`** on Linux and macOS: the traces match in both emulators, and so do the per-frame cycles.
- **Windows (MSVC)** builds and passes without SameBoy.

## Verification

```sh
cmake --workflow --preset linux-debug
build/linux-debug/tools/golem-run build/linux-debug/driver/scale.gb --expect tests/songs/scale.trace --emulator both --cycles
```
