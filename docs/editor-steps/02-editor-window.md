# Step 02: the editor window

## Goal

`golem-editor`, a desktop tracker window on top of the editing model (step 00) and live playback (step 01): open a song, edit it, play it, save it.

## Scope

In (`editor/`, SDL3 and Dear ImGui, both fetched with pinned versions):
- **Pattern grid:** 64 rows × 4 channels in tracker notation (`C-4 1 C0F`). The cursor is highlighted, and the rows of the beat (every 4 rows) are shaded.
- **Keys:**
  - arrows, Page Up/Down and Tab move the cursor;
  - note and hex keys enter values;
  - Delete clears;
  - `1` enters a note off.
- **Orders panel:** the list of orders with their four patterns. Select, insert and remove an order, and set or create a channel's pattern.
- **Toolbar:** Play/Stop (Space), octave, edit step, current instrument, ticks per row.
- **Menus and shortcuts:**
  - File: New (Ctrl+N), Open (Ctrl+O), Save (Ctrl+S), Save As (Ctrl+Shift+S), using SDL's native file dialogs;
  - Edit: Undo (Ctrl+Z), Redo (Ctrl+Y).
- **Title:** the file name, with `*` while there are unsaved changes.
- **Audio:** an SDL audio stream fed by `LivePlayer`. Without playback in the build, Play is greyed out and its tooltip gives the reason.
- **`GOLEM_EDITOR` option (on by default):** skips the editor and its dependencies when off.
- **Headless runs:** `--quit-after-frames N` quits after N frames and `--play` starts playback, for the `editor.smoke` test (SDL's dummy video and audio drivers). `--screenshot FILE.bmp` saves the last frame, so the window can be checked without a display.

Out: everything in the later steps of the [index](README.md).

## Acceptance criteria

- **The editor builds on every CI job,** and the `editor.smoke` test runs it headless there: it opens a song, starts playback and draws frames. Windows builds it without playback.
- **Manual checklist**, on Linux with SameBoy:
  1. open `tests/songs/scale.gsong`;
  2. Play and hear the scale;
  3. edit a note;
  4. undo, then redo;
  5. Save As, close and reopen: the edit is still there.
- **[composer.md](../composer.md)** describes the editor for users: its windows, keys and files.
