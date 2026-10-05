# Song format

All multi-byte values are 16-bit little-endian. Every block is reached through a header pointer, so block order does not matter.

## Header (19 bytes)

| Offset | Size | Field |
|---|---|---|
| 0 | 1 | Ticks per row (driver updates per row; 0 = 256) |
| 1 | 2 | Pointer to order count (1 byte) |
| 3 | 8 | Pointers to order tables, channels 1–4 |
| 11 | 2 | Pointer to pulse instruments |
| 13 | 2 | Pointer to wave instruments |
| 15 | 2 | Pointer to noise instruments |
| 17 | 2 | Pointer to waves |

## Orders

- **Order count** `N`: 1 byte. Playback loops back to order 0 after order `N-1`.
- **Order table** (one per channel): `N` pattern pointers.

## Patterns

64 rows × 3 bytes = 192 bytes.

| Byte | Bits 7–4 | Bits 3–0 |
|---|---|---|
| 0 | Note | Note |
| 1 | Instrument | Effect |
| 2 | Effect param `x` | Effect param `y` |

- **Note**: 0 = none, 1..72 = C-2..B-7. On channel 4 the note selects the noise clock shift/divider.
- **Instrument**: 0 = keep current, 1..15 = instrument 1..15. Channels 1–2 use the pulse table, channel 3 wave, channel 4 noise.

## Effects

Effects run on every tick except the row tick. `xx` is the whole parameter byte.

| Code | Effect | Parameter |
|---|---|---|
| 0 | Arpeggio | `x`, `y` = semitone offsets (`$00` = no effect) |
| 1 | Portamento up | `xx` added to period each tick |
| 2 | Portamento down | `xx` subtracted from period each tick |
| 3 | Tone portamento | Slide toward the row's note by `xx` each tick |
| 4 | Vibrato | `x` = ticks per change, `y` = period delta |
| 5 | Set master volume | `xx` → NR50 |
| 6 | Call routine | No-op |
| 7 | Note delay | Trigger note at tick `xx` |
| 8 | Set panning | `xx` → NR51 |
| 9 | Change timbre | Ch1–2: `xx` → NRx1 (duty/length). Ch3: `y` = wave index. Ch4: nonzero = 7-bit LFSR |
| A | Volume slide | `x` ≠ 0: volume up by `x`; else volume down by `y` |
| B | Position jump | Jump to order `xx` |
| C | Set volume | Sets envelope volume from `x`/`y` |
| D | Pattern break | Next order, row `xx` |
| E | Note cut | Cut note at tick `xx` |
| F | Set tempo | `xx` = ticks per row |

## Pulse instruments (15 × 3 bytes)

| Byte | Bits | Field |
|---|---|---|
| 0 | 7 | Length enable |
| 0 | 6–4 | Sweep pace |
| 0 | 3 | Sweep direction (1 = decrease) |
| 0 | 2–0 | Sweep steps |
| 1 | 7–6 | Duty cycle |
| 1 | 5–0 | Length timer |
| 2 | 7–4 | Envelope initial volume |
| 2 | 3 | Envelope increase |
| 2 | 2–0 | Envelope pace |

Byte 0 → NR10 (ch1 only), byte 1 → NRx1, byte 2 → NRx2.

## Wave instruments (15 × 2 bytes)

| Byte | Bits | Field |
|---|---|---|
| 0 | 7–0 | Length timer (→ NR31) |
| 1 | 7 | Length enable |
| 1 | 6–5 | Volume (NR32 code) |
| 1 | 4 | Reserved (0) |
| 1 | 3–0 | Wave index (0-based) |

## Noise instruments (15 × 2 bytes)

| Byte | Bits | Field |
|---|---|---|
| 0 | 7 | LFSR width |
| 0 | 6 | Length enable |
| 0 | 5–0 | Length timer (→ NR41) |
| 1 | 7–4 | Envelope initial volume |
| 1 | 3 | Envelope increase |
| 1 | 2–0 | Envelope pace |

Byte 1 → NR42.

## Waves (16 × 16 bytes)

32 4-bit samples per wave, two per byte, first sample in the high nibble. Copied as-is to wave RAM.
