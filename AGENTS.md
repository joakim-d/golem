# Project: Golem
This project allows a user to compose chiptune for the GameBoy system.

## Structure

- docs/ Everything related to documentation on how the project is structured, what is the song format, etc...
- core/ C++: fomat, reference player, editing model
- driver/ RGBDS asm
- tools/ Trace diff, emulator runner, wav export
- tests/songs Test songs and golden traces

## Commands

Requires CMake >= 3.25, a C++17 compiler, clang-format, and [RGBDS](https://rgbds.gbdev.io) (`rgbasm`, `rgblink`, `rgbfix` on `PATH`; without it the ROM is skipped with a warning). Linux and macOS presets use Ninja; Windows presets use Visual Studio 2022 (MSVC). Presets: `<os>-debug` / `<os>-release` with `<os>` in `linux`, `macos`, `windows`. Build output goes to `build/<preset>/`.

- Configure + build + test: `cmake --workflow --preset linux-debug`
- Configure: `cmake --preset linux-debug`
- Build: `cmake --build --preset linux-debug`
- Build only the GameBoy ROM: `cmake --build --preset linux-debug --target rom` (output: `build/linux-debug/driver/golem.gb`)
- Test: `ctest --preset linux-debug`
- Format: `cmake --build --preset linux-debug --target format`
- Format check: `cmake --build --preset linux-debug --target format-check`
- List presets: `cmake --list-presets=all`
- Reference player trace of a song: `build/linux-debug/tools/golem-trace tests/songs/minimal.gsong --frames 600` (`.gsong` is text; other files are binary songs, `--base` defaults to `0x4000`)
- Compare two traces: `build/linux-debug/tools/golem-tracediff expected.trace actual.trace` (exit 0 equal, 1 differ, 2 error)
- Regenerate golden traces after an intended player change: `GOLEM_UPDATE_GOLDEN=1 ctest --preset linux-debug -R Golden`, then review the `tests/songs/*.trace` diff

Third-party libraries are pulled with FetchContent: add a `cmake/<lib>.cmake` calling `golem_fetch_dependency()` (see `cmake/FetchDependency.cmake`), then `include(<lib>)` in the root `CMakeLists.txt`.

## Conventions

- When writing code use TDD, first add header files then tests then add implementation until the tests pass.
- C++ headers extensions are ".h", C++ implementation file extensions are ".cpp"

## Before you open a PR

- Compile the project, use clang-format and tests. All three must pass.

## Do not
- Modify the documentation without an approval