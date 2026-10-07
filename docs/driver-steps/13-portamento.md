# Step 13: portamento up and down (1, 2)

## Goal

1 `xx` and 2 `xx` slide the channel's period up or down by `xx` on every non-row tick of the row. The slid period stays the channel's period until the next note. As specified in the contract ("Effects", "Period and pitch") and implemented in the reference player first.

## Scope

In:
- **Portamento steps:** on channels 1–3, the channel's period + `xx` (1) or − `xx` (2), clamped to the note table (44 to 2015). When it changes, `NRx3` and `NRx4` are written without the trigger bit (with the length bit). Nothing before the channel's first note, nothing for `00`, nothing on channel 4.
- **The slid period stays:** there is no restore after the row. Retriggers (C, A, E) use its high bits. A new note resets it.
- **The arpeggio's base step** (tick % 3 = 0) and the row-tick restore use the channel's period, which may be slid, instead of the last note's period.

Out: tone portamento and vibrato (3, 4).

## Design notes

- **Pitch as a period:** the driver tracks each channel's sounding pitch as a 16-bit period (`wPitches`) instead of a note (`wPitchNotes`, step 12), because a slid period is not a note. Arpeggio steps, restores and portamento steps all compare and write periods through one routine, `WritePitch`.
- **`wPortas` and `wPortaDown`:** `wPortas` (the step size, 0 meaning none) is cleared with the other per-row effect tables. `wPortaDown` holds the direction and is only read while `wPortas` is non-zero.
- **Clamping** uses the first and last entries of the generated `NotePeriods` table, so it follows the reference player's `note_period()`.

## Results

All 17 driver traces pass. `driver.arpeggio` is unchanged by the move from note-based to period-based pitch tracking.

| Measure | Step 12 | Step 13 |
|---|---|---|
| `GolemPlay` worst case | 10424 cycles | 11012 cycles (79% of the budget) |
| Mean of the songs' averages | 780 cycles | 825 cycles |
| ROM / WRAM | 1711 / 89 bytes | 1892 / 101 bytes |

**ROM is at 92% of its budget.** Tone portamento (3) and vibrato (4) will not fit in the remaining 156 bytes, so the next step will have to raise the ROM budget or save space.

## Acceptance criteria

**`driver.portamento` passes.** The song (`tests/songs/portamento.gsong`, 600 frames) covers:
- both directions, and clamping at both ends;
- a slide that continues across rows, with no restore;
- a retrigger at the slid period, and a new note resetting it;
- an arpeggio on a slid channel;
- `00`, and a portamento before any note;
- the wave channel, and the noise channel (ignored);
- two channels sliding on the same tick.

All earlier driver tests (`driver.arpeggio` included) and the budgets still pass.

## Verification

```sh
cmake --workflow --preset linux-debug
ctest --preset linux-debug -R driver --output-on-failure
```
