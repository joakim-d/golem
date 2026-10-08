# Step 03: instrument and wave editors

## Goal

Edit a song's instruments and waves in the editor, instead of by hand in the `.gsong` file. You hear the changes on the next Play.

## Scope

In `core/` (namespace `golem::edit`, unit tests first):
- **Field codecs** (`golem/instrument_fields.h`): each instrument type decoded into named fields, and encoded back into its bytes ([song format](../song-format.md)).
  - Pulse: length enable and timer, sweep pace, direction and steps, duty, and envelope volume, direction and pace.
  - Wave: length enable and timer, volume code, wave index.
  - Noise: LFSR width, length enable and timer, and envelope volume, direction and pace.

  Encoding clamps each field to its range, so it always gives valid bytes.
- **Waves:** a sample (0–31, 4 bits) read and written by index; a wave as a hex line of 32 digits and back; presets for square, saw, triangle and sine.
- **`Document` operations:** set a pulse, wave or noise instrument (1–15), set a wave (0–15), and set one sample of a wave.
  - Each is a change, undoable like any other. Setting a value equal to the current one changes nothing.
  - **Merged undo:** consecutive changes to the same instrument or wave form one undo step, until `finish_edit()` or any other kind of change. Dragging a slider or drawing a wave is then one step.

In `editor/`:
- The left panel gets tabs: **Orders | Instruments | Waves**.
- **Instruments tab:** the type (Pulse, Wave, Noise) and the instrument number, which is the toolbar's current instrument. Each field has a slider, a checkbox or a list in musical terms: duty 12.5/25/50/75%, wave volume mute/100/50/25%, and so on. Sweep only acts on channel 1, which the tab says.
- **Waves tab:** the wave number, its 32 samples as bars to click or drag, its hex line to copy or paste, and the presets.
- An edit ends (`finish_edit()`) when no widget is active any more.

Out:
- hearing an instrument on its own, or edits during playback (a later step);
- copying instruments between songs.

## Acceptance criteria

- **`Edit.*` unit tests** cover:
  - a round trip through the field codecs for every byte value, and the clamping;
  - samples, hex lines (including invalid ones) and presets;
  - each new `Document` operation, unchanged values, merged undo steps and what ends them, and the modified flag.
- Every existing test passes, and so does `format-check`. The editor builds on every CI job.
- **Manual checklist,** on Linux with SameBoy:
  1. change instrument 1's duty and envelope, Play, and hear the difference;
  2. draw a wave and hear it on channel 3;
  3. undo, then redo, a slider drag and a wave drawing (one step each);
  4. save, then reopen: the instruments and waves are kept.
- **[composer.md](../composer.md)** describes the Instruments and Waves tabs.
