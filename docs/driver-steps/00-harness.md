# Step 00: test harness

## Goal

Run the driver headless in an emulator, log its APU writes frame by frame, and compare them with the golden trace of a song as a ctest test.

## Scope

In:
- **`golem-run`** (`tools/run_main.cpp`, library `tools/run/`) runs a ROM in Peanut-GB and records every write to `$FF10`–`$FF3F`.
- **Frames come from the ROM itself.** The test ROM writes to the unused address `$FF15` before `GolemInit` (frame 0) and before each `GolemPlay` call. Writes before the first marker are ignored, and markers are not part of the trace.
- **`golem-encode`** writes the binary song for `$4000`. **`golem-tables`** writes `notes.inc` from the reference player's note tables.
- **The test ROM shell** (`driver/test_rom.asm`): `GolemInit` with the song at `$4000`, then `GolemPlay` after every VBlank.
- **`golem_driver_test(<song>)`** in `driver/CMakeLists.txt` builds `build/<preset>/driver/<song>.gb` and adds the test `driver.<song>`.
- **The step 0 driver:** `GolemInit` makes the frame 0 writes; `GolemPlay` does nothing.

Out: any playback.

## Design notes

- **Peanut-GB v1.3.0 runs without sound emulation.** The comparison is on register writes, so how accurately the emulator would *sound* doesn't matter. Peanut-GB passes Blargg's CPU instruction and timing tests.
- **The emulator returns `$FF` for every APU register read.** The driver must never read APU registers back.
- **Peanut-GB's own reset writes** (`NR52=$F1`) happen outside a frame and are ignored.

## Acceptance criteria

- `driver.silence` (`tests/songs/silence.gsong`, 100 frames: the 3 init writes, then nothing) passes.
- **`RomRunner.*` unit tests pass:** frames split at markers, writes before the first marker ignored, incomplete frames reported, invalid ROMs rejected.
- **A deliberate driver bug is reported** with frame, write number, expected and actual values. For example, `NR50=$76` gives `frame 0: write #2: expected NR50=77, got NR50=76`.
- **Without RGBDS,** configuring prints a warning, the driver tests are skipped, and everything else passes.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R 'driver|RomRunner' --output-on-failure
```
