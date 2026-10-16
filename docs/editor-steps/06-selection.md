# Step 06: selection, copy and paste, transpose

## Goal

Work on blocks of the pattern instead of one field at a time: select rows and channels, cut, copy, paste and clear them, and transpose their notes.

## Scope

In `core/` (namespace `golem::edit`, unit tests first):
- **Fields:** a selection counts three fields per channel: the note, the instrument, and the effect (code and parameter together, so that a block never holds half an effect). Each cursor column belongs to one field.
- **Selection:** a block of the cursor's order, from an anchor to an end: a range of rows, and a range of fields across channels.
  - Moving the cursor with `select` set extends the selection: the anchor is where the cursor was, and the end follows the cursor. Moving it without `select` clears the selection.
  - **Select all:** every row and field of the order, without moving the cursor.
  - Changing the order, undo and redo clear the selection.
- **Clip:** a copied block: its rows, the fields it spans, and the field it starts with.
- **`Document` operations,** on the selection, or on the cursor's cell (all three fields) when nothing is selected:
  - **Copy:** returns a clip; not a change.
  - **Cut:** copy, then clear.
  - **Clear:** with a selection, empties its fields without moving the cursor. Without one, `clear()` works as before.
  - **Paste:** writes a clip at the cursor's row and channel, starting with the clip's first field kind (an effect goes into an effect field). Whatever falls past the last row or channel is dropped. The pasted block becomes the selection. An empty clip changes nothing.
  - **Transpose** by a number of semitones: the notes of the block's note fields. A pattern used by several selected channels is transposed once. If a note would leave C-2..B-7, nothing changes, so the intervals stay as they were.

  Each operation that changes the song is one undo step.

In `editor/`:
- **Selecting:** Shift with the arrows, Page Up/Down and Home/End; Shift+click; a mouse drag; Ctrl+A for the whole order; Escape clears it. The selected fields are shaded in their own colour, apart from the cursor.
- **Keys** while the pattern has the focus: Ctrl+X, Ctrl+C, Ctrl+V; Delete and Backspace clear the selection; Ctrl+Up/Down transposes by a semitone, Ctrl+Shift+Up/Down by an octave.
- **Edit menu:** Cut, Copy, Paste, Delete, Select All, and the four transpositions, with their shortcuts.
- The clip lives in the editor, so it survives New and Open: blocks can be copied between songs.
- **Shortcut letters follow the keyboard layout** (as Undo and Redo already do), so that Ctrl+A on AZERTY selects all instead of quitting.

Out:
- inserting and deleting rows (shifting the rest of the pattern);
- mix paste (only the non-empty fields), interpolation, and scaling volumes;
- the system clipboard (sharing blocks between two editor windows);
- selections across orders.

## Acceptance criteria

- **Unit tests,** `Edit.*`:
  - the selection: extending in each direction and across channels, clearing it, select all, a change of order;
  - copy, cut and clear of blocks, with and without a selection;
  - paste: field alignment, clipping at the last row and channel, the pasted block selected, between two songs;
  - transpose: up and down, a refusal at the range ends, only note fields, empty notes left alone, a shared pattern once;
  - undo and redo of each operation.
- Every existing test passes, and so does `format-check`. The editor builds on every CI job.
- **Manual checklist,** on Linux:
  1. select a block with Shift+arrows, then with the mouse, and copy it into another pattern;
  2. cut a block and undo;
  3. transpose a melody by an octave and hear it;
  4. copy a block, open another song, and paste it there.
- **[composer.md](../composer.md)** describes selecting, the clipboard keys and transposing.
