# Project: Golem
This project allows a user to compose chiptune for the GameBoy system.

## Structure

- docs/ Everything related to documentation on how the project is structured, what is the song format, etc...
- core/ C++: fomat, reference player, editing model
- driver/ RGBDS asm
- tools/ Trace diff, emulator runner, wav export
- tests/songs Test songs and golden traces

## Commands

Requires CMake >= 3.25, a C++17 compiler, clang-format, and [RGBDS](https://rgbds.gbdev.io) (`rgbasm`, `rgblink`, `rgbfix` on `PATH`; without it the driver ROMs and tests are skipped with a warning). All presets use Ninja; Windows presets compile with MSVC (`cl`), so run them from a Developer PowerShell or Developer Command Prompt (any Visual Studio version), or open the folder in Visual Studio. Presets: `<os>-debug` / `<os>-release` with `<os>` in `linux`, `macos`, `windows`. Build output goes to `build/<preset>/`.

- Configure + build + test: `cmake --workflow --preset linux-debug`
- Configure: `cmake --preset linux-debug`
- Build: `cmake --build --preset linux-debug`
- Build only the driver test ROMs: `cmake --build --preset linux-debug --target driver-roms` (output: `build/linux-debug/driver/<song>.gb`)
- Test: `ctest --preset linux-debug`
- Driver tests only (driver trace vs golden trace and cycle budget, one per song, plus `driver.size`): `ctest --preset linux-debug -R driver --output-on-failure`
- Format: `cmake --build --preset linux-debug --target format`
- Format check: `cmake --build --preset linux-debug --target format-check`
- List presets: `cmake --list-presets=all`
- Reference player trace of a song: `build/linux-debug/tools/golem-trace tests/songs/minimal.gsong --frames 600` (`.gsong` is text; other files are binary songs, `--base` defaults to `0x4000`)
- Compare two traces: `build/linux-debug/tools/golem-tracediff expected.trace actual.trace` (exit 0 equal, 1 differ, 2 error)
- Driver trace of a test ROM: `build/linux-debug/tools/golem-run build/linux-debug/driver/scale.gb --frames 100`, or compare with a golden trace: `--expect tests/songs/scale.trace` (exit 0 match, 1 differ, 2 error); `--cycles` reports the cycles of `GolemInit` and `GolemPlay`, `--max-cycles N` fails above N, `--emulator peanut|sameboy|both` picks the emulator (`both` also requires identical traces and cycles; SameBoy is built with GCC or Clang only). Budgets live in `driver/CMakeLists.txt` and `docs/driver-steps/07-budgets.md`
- Listen to the driver songs: `cmake --build --preset linux-debug --target driver-wavs` (one WAV per driver song in `build/linux-debug/driver/`, as long as its golden trace), or `build/linux-debug/tools/golem-wav <rom.gb> out.wav [--frames N] [--rate HZ]`. Rendered by SameBoy (GCC or Clang builds only); for ears only, never a test criterion
- Encode a text song to binary: `build/linux-debug/tools/golem-encode song.gsong song.bin --base 0x4000`
- Regenerate golden traces after an intended player change: `GOLEM_UPDATE_GOLDEN=1 ctest --preset linux-debug -R Golden`, then review the `tests/songs/*.trace` diff
- Driver work follows the steps in `docs/driver-steps/`; a song becomes a driver test with `golem_driver_test(<song>)` in `driver/CMakeLists.txt`

Third-party libraries are pulled with FetchContent: add a `cmake/<lib>.cmake` calling `golem_fetch_dependency()` (see `cmake/FetchDependency.cmake`), then `include(<lib>)` in the root `CMakeLists.txt`.

## Conventions

- When writing code use TDD, first add header files then tests then add implementation until the tests pass.
- C++ headers extensions are ".h", C++ implementation file extensions are ".cpp"

## Before you open a PR

- Compile the project, use clang-format and tests. All three must pass.

## Do not
- Modify the documentation without an approval