# Step 01: live playback

## Goal

Play the song being edited with the real driver in SameBoy, producing audio sample by sample for the editor's audio device, with no RGBDS needed when the editor runs.

## Scope

In:
- **Player ROM template (`player.gb`).** Built at compile time from the test ROM shell, the driver, and an empty song slot at `$4000`. CMake turns it into a byte array compiled into the library.
- **Patching:** `encode_song(song, 0x4000)` is copied into a copy of the template at `$4000`. A song bigger than the bank (`$4000`–`$7FFF`, 16 KiB) can't be played, and the error says so.
- **`LivePlayer`** (`tools/run/include/golem/live_player.h`):
  - `play(song)` patches and starts the ROM in SameBoy from power-on;
  - `render(samples)` fills a buffer with stereo samples (silence when stopped);
  - `stop()` stops it, and `is_playing()` reports the state;
  - `available()` reports whether playback is built in, with the reason when it isn't.

Out:
- playing from the cursor (a later, oracle-first step);
- hearing edits while the song plays.

## Design notes

- **Why the trace test is enough:** a patched template is a test ROM like any driver test's, so it can be checked with `run_rom`. If its trace equals the golden trace, the template and the patching are correct, and what plays is what the tests verified.
- **Reuse:** `LivePlayer` reuses the `sameboy_core` glue and `render_audio`'s audio callback.

## Acceptance criteria

- **Every golden song, patched into the template, has a trace equal to its golden trace** (`run_rom`, Peanut-GB and SameBoy).
- **`LivePlayer.*` tests:** silence before `play`; sound after `play` on a song with notes; silence after `stop`; a song too big for the bank is refused; and `available()` matches the build.
- **Without SameBoy or RGBDS,** the editor library still builds, and `available()` explains why playback is missing.
