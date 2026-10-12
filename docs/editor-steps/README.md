# Editor steps

The Golem editor is a desktop tracker (SDL3 and Dear ImGui) for writing songs in the format of [song-format.md](../song-format.md). It plays them with the real driver in SameBoy, so what you hear is what a Game Boy plays. It is built step by step like the driver ([driver-steps](../driver-steps/README.md)): each step has a plan file with its acceptance criteria.

| Step | Adds | Acceptance | Status |
|---|---|---|---|
| [00](00-editing-model.md) | Editing model in `core/`: document, cursor, note and hex entry, orders, undo/redo, open/save | `Edit.*` tests | Done |
| [01](01-live-playback.md) | Player ROM template and `LivePlayer`: a song played by the driver in SameBoy, sample by sample | `LivePlayer.*` tests; golden songs patched into the template reproduce their golden traces | Done |
| [02](02-editor-window.md) | The editor window: pattern grid, orders, toolbar, menus, audio, file dialogs | Builds on every CI job; manual checklist; [composer.md](../composer.md) user guide | Built; manual checklist to do |
| [03](03-instrument-editors.md) | Instrument and wave editors: field codecs, wave tools, merged undo steps; Instruments and Waves tabs | `Edit.*` tests; manual checklist; [composer.md](../composer.md) section | In progress |
| [04](04-play-feedback.md) | Play feedback: note preview, playing row, Follow | `Player.*`, `Edit.*`, `LivePlayer.*` tests; manual checklist; [composer.md](../composer.md) section | In progress |

Milestone 1 (edit, play, save) is steps 00–02.

## Rules for every step

- **Logic stays out of the UI.** Behaviour that can be tested headless lives in `core/` or `tools/run/`, with unit tests written first (header, tests, then implementation). The `editor/` app maps windows, keys and the audio device onto those APIs.
- **Acceptance:** the step's tests pass, and so does everything else. The build, `ctest` and `format-check` pass.
- **The song format and the driver stay as they are,** unless a step goes through the driver's "oracle first" process: contract, reference player and golden trace, then the driver.

## Later steps (outline)

- Play from the cursor. This needs a driver entry point that starts at an order and row, so it goes oracle first.
- Export: playable ROM (`.gb`), WAV, binary song.
- Selection, copy and paste, transpose.
- Live edits heard while the song plays.
