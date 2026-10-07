# Golem

Golem is a tool for composing chiptune music for the Nintendo Game Boy. It is made of a song format, a C++ editing core, and a sound driver written in Game Boy assembly (RGBDS).

## About this project

Golem is a **training project for harness engineering**: building a codebase together with coding agents, where the tooling around the code (tests, reference implementations, trace diffs, conventions) is what keeps the agent's work correct. It is inspired by [Golem](https://gitlab.com/joakim34/golem), an existing Game Boy music project that I wrote by hand. Golem rebuilds the same idea from scratch, this time with agents doing most of the coding.

### How correctness is defined

In Golem, the definition of correct behaviour exists before the driver code that has to meet it:

1. A **reference player** in C++ (`core/`) turns a song into the exact APU register writes of every frame. [docs/driver-contract.md](docs/driver-contract.md) describes its behaviour.
2. The assembly **driver** is checked against that player mechanically. A test ROM plays a song in an emulator that logs APU writes, and the log is diffed frame by frame against the reference trace:

   ```
   frame 212: write #2: expected NR22=F3, got NR22=F1
   ```

3. WAV rendering is planned, but only as a check for human ears. It will never decide whether a test passes.

## Status

| Part | State |
|---|---|
| Song format ([docs/song-format.md](docs/song-format.md)) | Specified |
| Song model, text (`.gsong`) and binary loaders | Done |
| Reference player: notes, instruments, effects 5 6 8 9 B C D F | Done |
| Reference player: effects 0–4, 7, A, E | To do |
| Trace tools (`golem-trace`, `golem-tracediff`) and golden traces | Done |
| Emulator harness (`golem-run`, Peanut-GB) and driver test ROMs | Done |
| Sound driver (RGBDS), grown step by step ([docs/driver-steps](docs/driver-steps/README.md)) | Step 1 of 7: channel 1 |
| Cycle and size budgets | To do |
| Editor, WAV export | To do |

## Layout

```
core/         C++ library: song format, reference player, traces (+ unit tests)
driver/       Game Boy sound driver, RGBDS assembly
tools/        Command-line tools (golem-trace, golem-tracediff, golem-run, golem-encode)
tests/songs/  Test songs (.gsong) and their golden traces (.trace)
docs/         Song format, driver contract, driver steps
cmake/        CMake helpers (dependency fetching, GoogleTest)
```

## Building

You need:
- CMake 3.25 or newer
- A C++17 compiler
- clang-format
- [RGBDS](https://rgbds.gbdev.io), to build the Game Boy ROM. Without it, the ROM is skipped and you get a warning.

GoogleTest is downloaded automatically.

```sh
cmake --workflow --preset linux-debug     # configure, build and run all tests
```

Presets also exist for `macos-*` and `windows-*` (Debug and Release). Run `cmake --list-presets=all` to see them all.

Print the reference trace of a song:

```sh
build/linux-debug/tools/golem-trace tests/songs/minimal.gsong --frames 600
```

[AGENTS.md](AGENTS.md) has the full list of commands and the project conventions. It is written for coding agents, and is just as useful for humans.
