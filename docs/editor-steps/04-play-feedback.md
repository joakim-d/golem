# Step 04: play feedback

## Goal

Hear a note as you enter it, and see where the song is while it plays.

## Scope

In `core/` (unit tests first):
- **`Player::position()`:** the order, row and tick of the frame the reference player last stepped. The driver plays the same frames as the reference player (the golden traces check it), so this is also the driver's position.
- **`edit::preview_song()`:** a song that plays one note with one instrument on one channel: the song's instruments and waves, one order, and the note on the first row of that channel's pattern. Its row lasts 256 ticks, so the note is not retriggered.

In `tools/run/` (unit tests first):
- **`LivePlayer::position(latency)`:** while a song plays, the reference player follows the driver frame by frame, by the frame markers of the player ROM. The position is that of the sample `latency` samples before the last one rendered, which is the one being heard while `latency` samples wait in the audio output. There's no position when nothing plays.
- **`mix_into()`:** adds one stream of samples to another, saturating, so that two players can be heard at once.

In `editor/`:
- **Note preview:** entering a note plays it, with the current instrument, on the cursor's channel, through a second `LivePlayer` mixed with the song. It stops when the key is released, after 3 seconds at most. A held key's repeats don't restart it. It works while the song plays.
- **Playing row:** while the song plays, its current row is shaded in the pattern, if the pattern shows the playing order. The delay of the audio output is taken into account.
- **Follow** (toolbar, on by default): while the song plays, the pattern shows the playing order and scrolls with the playing row.

Out:
- playing from the cursor (it needs a driver entry point);
- previewing a whole row or the effects of a cell.

## Acceptance criteria

- **Unit tests:**
  - `Player.*`: the position through rows, ticks per row, order changes, jumps (`B`), breaks (`D`) and tempo changes (`F`);
  - `Edit.*`: the preview song, for each channel type;
  - `LivePlayer.*`: the position while playing, with and without latency, after a restart, and none when stopped; `mix_into()` saturation.
- Every existing test passes, and so does `format-check`. The editor builds on every CI job.
- **Manual checklist,** on Linux with SameBoy:
  1. enter notes on each channel and hear each one, also while the song plays;
  2. hold a key: the note sustains until the key is released;
  3. play `scale.gsong`: the shaded row follows the notes you hear;
  4. turn Follow off: the pattern stays where it is.
- **[composer.md](../composer.md)** describes the preview, the playing row and Follow.
