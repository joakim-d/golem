# Step 05: export

## Goal

Take a song out of the editor: as a Game Boy ROM that plays it, as a WAV file to listen to or share, and as a binary song to build into a game with the driver.

## Scope

In `core/` (unit tests first):
- **`song_length()`:** the frames of one pass of a song. Playback goes from frame 1 until it reaches the start of a row it has already played: that row is the loop point. A song whose last order loops back to order 0 has every row in its pass. A song that ends with `B01` has order 0 as an intro, played once.

In `tools/run/` (unit tests first, `golem/export.h`):
- **`render_song()`:** the audio of a number of passes of a song, then a tail (the release of the last notes), as the driver plays it in SameBoy.
- **`export_rom()`, `export_wav()`, `export_binary()`:** write a song to a file:
  - the player ROM with the song (`player_rom()`), which plays it from power-on on a Game Boy or in an emulator;
  - a 16-bit stereo WAV of the passes and a 2-second tail;
  - the encoded song at `$4000` (`encode_song()`), the layout the player ROM uses.

  A file that cannot be written throws `RunError`.

In `editor/`:
- **File > Export ROM, Export WAV, Export Binary Song,** each through a save dialog, with the song's name and the right extension.
- **Export WAV** first asks for the number of passes (1–4), and shows the length it gives. WAVs over 20 minutes are refused.
- Without playback in the build, Export ROM and Export WAV are greyed out with the reason. Export Binary Song works in every build.

Out:
- exporting from the command line (the editor, and `golem-encode` for binary songs);
- fades and other audio processing;
- choosing the binary song's address (always `$4000`).

## Acceptance criteria

- **Unit tests:**
  - `Player.*`: song lengths with several orders, ticks per row, tempo changes, pattern breaks, a jump back to a later order (an intro), and a jump to order 0;
  - `Export.*`: the length of rendered audio, the files written (ROM bytes, WAV header and length, binary song bytes), and an unwritable path.
- Every existing test passes, and so does `format-check`. The editor builds on every CI job.
- **Manual checklist,** on Linux with SameBoy:
  1. export a ROM and play it in an emulator;
  2. export a WAV of 2 passes and listen to it;
  3. export a binary song and compare it with `golem-encode` of the saved song.
- **[composer.md](../composer.md)** describes exporting.
