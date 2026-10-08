# Step 00: editing model

## Goal

Everything the editor does to a song, as a library with no UI: a document with a cursor, entry of notes and hex digits, orders, undo and redo, and opening and saving `.gsong` files. The window (step 02) only turns keys and clicks into these calls.

## Scope

In (`core/include/golem/edit.h`, namespace `golem::edit`):
- **`Document`:** the `Song`, the file path, and a "modified" flag that turns off again after a save, or after undoing back to the saved state.
- **A new document** is ready to play. It has:
  - one order using patterns 0–3 (one per channel, so editing one channel never changes another);
  - 6 ticks per row;
  - an audible instrument 1 for each channel type;
  - wave 0 set to a triangle.
- **Cursor:** order, row, channel and column. A cell has five columns: note, instrument, effect code, and the two parameter digits. Rows clamp to 0–63. Columns move across channels: right from channel 1's last digit goes to channel 2's note.
- **Note entry** with a tracker keyboard map (QWERTY):
  - the lower row (`z s x d c v g b h n j m , l . ; /`) plays from C of the current octave;
  - the upper row (`q 2 w 3 e r 5 t 6 y 7 u i 9 o 0 p`) plays from C of the octave above.

  The note gets the current instrument. Notes outside C-2..B-7 are refused.
- **Note off:** effect `C00` with no note. It is the note off used throughout the driver work.
- **Hex entry:**
  - in the instrument column, an instrument (`0` meaning none);
  - in the effect columns, the effect code or one digit of the parameter.
- **Clear** empties the field under the cursor: note and instrument, instrument, or effect and parameter.
- **Edit step:** every entry moves the cursor down that many rows; 0 keeps it in place.
- **Patterns are shared:** an edit applies to the cell's pattern, wherever it is used.
- **Orders:**
  - insert a copy of the current order after it;
  - remove the current order, keeping at least one;
  - set a channel's pattern (creating empty patterns up to that index);
  - give a channel a new empty pattern.

  The song is limited to 255 orders and 256 patterns.
- **Song settings:** ticks per row.
- **Undo and redo** of every change. Each change keeps a snapshot of the song and the cursor (a few KB), and a new change clears the redo history.
- **Open and save** through `parse_song_text` and `format_song_text`. A file that doesn't parse, or isn't a valid song, throws `SongError`. A save writes the file and marks the document unmodified.

Out:
- instrument and wave editing;
- selection and clipboard;
- playback (step 01).

## Acceptance criteria

**`Edit.*` unit tests cover:**
- the keyboard map and octaves;
- the new document;
- every kind of entry, including edit steps 0 and above;
- refused notes;
- cursor movement and clamping;
- shared patterns;
- every order operation and its limits;
- undo and redo, restoring both the song and the cursor;
- the modified flag through edits, saves and undos;
- an open/save round trip, and errors when opening.

Every existing test still passes, and `format-check` passes.
