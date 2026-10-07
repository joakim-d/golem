# Driver contract

What the sound driver must write to the APU, frame by frame, for a song in the format of [song-format.md](song-format.md).

The executable definition is the reference player in `core/` (`golem::Player`, [core/src/player.cpp](../core/src/player.cpp)). The driver is correct when its APU trace equals the reference player's trace (see [Traces](#traces)). If this document and the reference player disagree, one of them has a bug, and it must be fixed before either is trusted.

## Driver interface

The driver ([driver/golem.inc](../driver/golem.inc)) has two entry points:

- **`GolemInit`**: `hl` = address of the song. Starts the song at order 0, row 0 and makes the frame 0 writes.
- **`GolemPlay`**: plays one frame. Called once per frame (VBlank) after `GolemInit`.

Both clobber `af`, `bc`, `de` and `hl`.

- **The song must stay mapped** at the address it was encoded for (binary song pointers are absolute).
- **State:** the driver keeps its state in WRAM0.
- **No APU reads:** the driver never reads APU registers. Their read values differ from what was written, and the test harness does not emulate them.

## Frames

- **Frame 0** is the `init` call. It writes, in order: `NR52=$80`, `NR50=$77`, `NR51=$FF`.
- **Frame n ≥ 1** is the n-th `play` call, one per VBlank.
- The first `play` call is the row tick of order 0, row 0.
- A row lasts `ticks_per_row` frames (0 means 256). Its first frame is the **row tick**; the others are the **non-row ticks**.
- After the last row of the last order, playback continues at order 0, row 0. The song never ends.

## Row tick

Channels are processed in order 1, 2, 3, 4. For each channel, in this order:

1. **Instrument column.** A nonzero instrument becomes the channel's current instrument. Nothing is written. At the start, every channel's current instrument is 1.
2. **Note.** A note triggers the channel with the current instrument (see [Triggers](#triggers)).
3. **Effect.** The row's effect is applied (see [Effects](#effects)).

Position jumps and pattern breaks are resolved after all four channels (see [Flow control](#flow-control)).

## Triggers

A trigger always writes the full set of registers below, in this order, even if a value did not change. `len` is the current instrument's length-enable bit, placed in bit 6 of NRx4 (`$40`), and `period` is the note's period.

| Channel | Writes |
|---|---|
| 1 | `NR10`=pulse byte 0 & `$7F`, `NR11`=byte 1, `NR12`=byte 2, `NR13`=period low, `NR14`=`$80` \| len \| period bits 10–8 |
| 2 | `NR21`=byte 1, `NR22`=byte 2, `NR23`=period low, `NR24`=`$80` \| len \| period bits 10–8 |
| 3 | [wave load], `NR30`=`$80`, `NR31`=length timer, `NR32`=volume code, `NR33`=period low, `NR34`=`$80` \| len \| period bits 10–8 |
| 4 | `NR41`=length timer, `NR42`=envelope, `NR43`=noise value \| width, `NR44`=`$80` \| len |

- **Channel 2** uses the pulse instrument table, but byte 0 contributes only its length-enable bit.
- **Wave load (channel 3).** If the wanted wave differs from the one in wave RAM, the driver writes `NR30=$00`, then the 16 bytes of the wave to `$FF30`–`$FF3F` in address order. At start-up no wave is loaded. If the wave is already loaded, nothing is written.
- **Width (channel 4).** `$08` when the LFSR is 7-bit (instrument byte 0 bit 7), otherwise `$00`.

### Note → period

Note 1 is C-2 (MIDI 36), and pitch uses equal temperament with A-4 = 440 Hz:

```
f(note) = 440 · 2^((note + 35 − 69) / 12)
period  = round(2048 − 131072 / f(note))      (round half away from zero)
```

For example, C-2 = 44, A-4 = 1750, B-7 = 2015.

Channel 3 uses the same table, so it sounds one octave lower than channels 1 and 2.

### Note → noise (channel 4)

Each note maps to the NR43 clock shift `s` (0–13, bits 7–4) and divider `r` (0–7, bits 2–0) whose noise frequency `262144 / (r = 0 ? 0.5 : r) / 2^s` is nearest to `f(note)` on a log scale. On a tie, the smaller shift wins. For example, A-4 → `$75`, C-5 → `$74`.

The tables come from `golem::note_period()` and `golem::noise_nr43()` in [core/include/golem/notes.h](../core/include/golem/notes.h).

## Effects

An empty effect cell is effect `0` with parameter `$00`.

Effects fall into three groups by when they act. This refines "Effects run on every tick except the row tick" in song-format.md:

- **One-shot** (5, 6, 8, 9, B, C, D, F) act on the row tick.
- **Continuous** (0–4, A) act on the non-row ticks.
- **Timed** (7, E) act at tick `xx` of the row.

| Effect | Without a note on the row | With a note on the row |
|---|---|---|
| 5 Set master volume | `NR50=xx` | After the trigger: `NR50=xx` |
| 6 Call routine | Nothing | Nothing |
| 8 Set panning | `NR51=xx` | After the trigger: `NR51=xx` |
| 9 Change timbre, ch1–2 | `NRx1=xx` | The trigger writes `xx` to `NRx1` |
| 9 Change timbre, ch3 | Wave `y`: [wave load], `NR30=$80`, `NR34`=`$80` \| len \| period bits 10–8. Nothing if wave `y` is already loaded | The trigger uses wave `y` |
| 9 Change timbre, ch4 | `NR43`=noise value of the last note \| (`xx`≠0 ? `$08` : 0) | The trigger uses width `xx`≠0 |
| C Set volume, ch1, 2, 4 | `NRx2=xx`, then `NRx4`=`$80` \| len \| period bits 10–8 (`$80` \| len on ch4) | The trigger writes `xx` to `NRx2` |
| C Set volume, ch3 | `NR32`=(`x` & 3) << 5 | The trigger writes (`x` & 3) << 5 to `NR32` |
| B, D, F | See [Flow control](#flow-control) | Same |

Further rules:

- **9 and C last one note.** Their values are not remembered: the next trigger takes the instrument's values again.
- **No-note fallbacks.** When there is no previous note, "period" and "noise value of the last note" are 0.
- **Not specified yet:** effects 0–4, 7, A and E (all except an empty cell). The reference player rejects them (`golem::UnsupportedEffect`).

## Flow control

- **B `xx`**: after this row, continue at order `xx`, row 0.
- **D `xx`**: after this row, continue at the next order, row `xx`.
- **B and D on the same row** (any channels): continue at order B, row D.
- **Several channels using B, or several using D:** the highest channel wins.
- **Out of range:** an order ≥ the order count becomes 0, and so does a row ≥ 64. D on the last order wraps to order 0.
- **F `xx`**: ticks per row = `xx` (0 = 256), starting from the next row. The current row keeps its length. The tempo stays in effect when the song loops.

## Traces

A trace is the ordered list of APU writes of each frame, in this text form (`.trace`):

```
golem-trace 1 frames=600
0000 NR52 80
0001 NR22 F3
```

The header gives the number of frames. Each following line is one write: `<frame> <register> <value>`, with the frame in decimal (at least 4 digits), the register name (`NR10`…`NR52`, `WAVE0`…`WAVEF` for wave RAM), and the value in hex. Frames with no writes have no lines.

**Pass criterion:** for every frame, the driver's writes are the same registers with the same values in the same order, and the traces cover the same number of frames. `golem-tracediff` reports the first difference, for example `frame 212: write #2: expected NR22=F3, got NR22=F1`.

Golden traces for the songs in `tests/songs/` are produced by the reference player.

### Test harness

`golem-run` produces the driver's trace by running a test ROM ([driver/test_rom.asm](../driver/test_rom.asm)) headless and logging every write to `$FF10`–`$FF3F`.

- **Frame markers:** the test ROM marks frames itself by writing to the unused address `$FF15`, once before `GolemInit` (frame 0) and once before each `GolemPlay` call. So trace frames match the driver's calls, whatever the emulator's timing.
- **Ignored writes:** writes before the first marker, and the markers themselves.

The plan for growing the driver, step by step, is in [driver-steps/](driver-steps/README.md).
