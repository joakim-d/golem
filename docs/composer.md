# Golem editor

`golem-editor` is a tracker for Game Boy music. You write a song as patterns of notes and effects, and hear it played by the real Golem driver in an emulated Game Boy (SameBoy), exactly as a Game Boy would play it. Songs are saved as `.gsong` text files ([song format](song-format.md)).

## Starting

```sh
cmake --workflow --preset linux-debug          # builds golem-editor with the rest
build/linux-debug/editor/golem-editor [song.gsong]
```

A new song is ready to play: four channels, each with its own empty pattern, and an audible instrument 1 for each channel type (pulse, wave, noise). Notes you enter sound straight away.

Playback needs a build with SameBoy (GCC or Clang) and RGBDS. Without them, the editor still edits and saves, and the Play button's tooltip says what is missing.

## The window

- **Toolbar**
  - **Play / Stop** plays the song from the beginning, with what you have written so far. Space does the same while the pattern has the focus.
  - **Octave** is the octave of the note keys' lower row.
  - **Edit step** is how many rows the cursor moves down after each entry (0 stays in place).
  - **Instrument** is the instrument given to the notes you enter.
  - **Ticks per row** is the song's speed: frames per row, 6 by default. Lower is faster.
- **Orders** (left): the song plays its orders from top to bottom, then starts over. Each order picks one pattern per channel (shown in hex).
  - **Insert** adds a copy of the selected order after it.
  - **Remove** deletes it (one order always remains).
  - **Pattern** sets the pattern of the cursor's channel in the selected order (type a hex number, then Enter).
  - **New** gives that channel a new empty pattern.

  Patterns are shared: editing a pattern changes it wherever it is used.
- **Pattern** (right): the 64 rows of the selected order, one column per channel. A cell reads `C-4 1 C0F`: note, instrument, effect code and its two-digit parameter. `---` and `.` mean empty. Click a field to put the cursor there; every 4th row is shaded.

## Keys (while the pattern has the focus)

| Keys | Action |
|---|---|
| Arrows | Move the cursor (left and right go through the fields and across channels) |
| Page Up / Page Down | 16 rows up / down |
| Home / End | First / last row |
| Tab / Shift+Tab | Next / previous channel |
| Delete / Backspace | Clear the field: note and instrument, instrument, or effect and parameter |
| Space | Play / Stop |

**Notes** (in the note column) are played by key *position*, like a piano, on any keyboard layout. These are the positions of a US QWERTY keyboard:

```
 2 3   5 6 7   9 0          <- upper row: C#, D#, F#, G#, A#, C#, D# (octave + 1)
Q W E R T Y U I O P         <- C D E F G A B C D E (octave + 1)
 S D   G H J   L ;          <- lower row: C#, D#, F#, G#, A#, C#, D# (octave)
Z X C V B N M , . /         <- C D E F G A B C D E (octave)
```

The note gets the current instrument. `1` enters a note off (effect `C00`: volume 0).

**Hex digits** (in the instrument and effect fields) are 0–9 on the number row or keypad, and A–F as letters.

## Menus and shortcuts

| Shortcut | Action |
|---|---|
| Ctrl+N | New song |
| Ctrl+O | Open a `.gsong` file |
| Ctrl+S / Ctrl+Shift+S | Save / Save As |
| Ctrl+Z | Undo (also Edit menu) |
| Ctrl+Y or Ctrl+Shift+Z | Redo |
| Ctrl+Q | Quit |

On macOS, Cmd works like Ctrl. Undo and redo cover every change to the song, and put the cursor back where it was. The window title shows the file name, with `*` while there are unsaved changes. New, Open and Quit ask before discarding them.

## Effects

The effect column takes the 16 effects of the [song format](song-format.md). The driver's exact behaviour is in the [driver contract](driver-contract.md).

| | | | |
|---|---|---|---|
| `0xy` arpeggio | `1xx` / `2xx` portamento up / down | `3xx` tone portamento | `4xy` vibrato |
| `5xx` master volume | `6xx` (no-op) | `7xx` note delay | `8xx` panning |
| `9xx` timbre | `Axy` volume slide | `Bxx` jump to order | `Cxy` set volume (`C00`: note off) |
| `Dxx` break to row | `Exx` note cut | `Fxx` ticks per row | |

## Not yet

- Editing instruments and waves (a song's instruments can be edited in the `.gsong` file for now).
- Playing from the cursor, and hearing edits while the song plays.
- Selection and copy/paste.
- Exporting a ROM or a WAV from the editor (`golem-wav` renders a driver test ROM).
