# Step 07: budgets

## Goal

Measure what the driver costs (CPU time per call, ROM and WRAM size) and make the tests fail when it grows past agreed limits.

## Scope

In:
- **Cycles per call.** The test ROM writes an end marker (`$FF27`, unused) right after `GolemInit` and after each `GolemPlay` returns. `golem-run` reads the emulator's cycle counter at each frame marker and end marker, and reports per frame the cycles of the driver call: `call`, the routine, and its `ret`.
- **`golem-run --cycles`** prints `GolemInit`'s cycles, and the maximum (with its frame) and average for `GolemPlay`. **`--max-cycles N`** fails the run when any `GolemPlay` call goes over `N`.
- **Size:** the driver's ROM section (`"Golem driver"`, code plus note tables) and WRAM section (`"Golem state"`), read from the `rgblink` map file.
- **Limits** chosen from the measured values plus headroom (see [Limits](#limits)), and enforced by ctest.

Out:
- optimising the driver; this step only measures it and sets the limits.

## Design notes

- **Clock.** Peanut-GB adds an instruction's cycles after running it, so at a marker write its clock (`DIV` << 8 | `div_count`, 16 bits) shows the time *before* the marker instruction.
- **What a frame's count covers.** Its marker instruction (`ldh`, 12 cycles), the `call`, the routine, and its `ret`. The runner subtracts the 12 cycles of the `ldh`.
- **Units.** Cycles are T-cycles at 4.194304 MHz; a frame is 70224.
- **No wrap-around check.** The clock is 16 bits, which is enough for any call shorter than a frame.

## Acceptance criteria

- **`RomRunner.*` tests cover cycle measurement** on hand-assembled ROMs with known instruction timings.
- **Cycle limit:** every `driver.<song>` test also checks `GolemPlay` against the cycle limit.
- **Size limits:** a `driver.size` test checks the ROM and WRAM sections against their limits.
- **Recorded:** the measured values and the chosen limits are written below.

## Limits

**Measured** at the end of step 06, with `golem-run --cycles` and the map file:

| Measure | Value |
|---|---|
| `GolemPlay`, worst case | 9956 cycles, 14.2% of a frame (`minimal`, frame 1: four triggers and a wave load on one row tick) |
| `GolemPlay`, average per song | 330–1000 cycles |
| `GolemPlay`, row tick of an empty song | 2896 cycles, mostly re-reading pointers from the song header |
| `GolemInit` | 664 cycles |
| ROM (`"Golem driver"`) | 1090 bytes, including 216 bytes of note tables |
| WRAM (`"Golem state"`) | 32 bytes |

**After the step 11 optimization**, with all of step 10's features:

| Measure | Step 10 | Step 11 |
|---|---|---|
| `GolemPlay`, worst case | 11792 cycles | 9688 cycles (−18%) |
| `GolemPlay`, mean of the songs' averages | 1084 cycles | 660 cycles (−39%) |
| `GolemPlay`, row tick of an empty song | 3512 cycles | 1624 cycles (−54%) |
| ROM | 1392 bytes | 1454 bytes |
| WRAM | 53 bytes | 75 bytes (pointer caches) |

**Enforced:** a target budget with room for the effects not specified yet (0–4, 7, A, E). The values are set in [driver/CMakeLists.txt](../../driver/CMakeLists.txt).

| Budget | Limit |
|---|---|
| `GolemPlay` (every call of every driver test) | 14000 cycles, about 20% of a frame |
| ROM | 3072 bytes (2048 until step 14) |
| WRAM | 128 bytes (64 until step 10) |

**Raising a limit** is a deliberate change: update it here and in `driver/CMakeLists.txt` in the same PR, with the reason.

### Changes

| Step | Budget | From → to | Reason |
|---|---|---|---|
| 10 (volume slide) | WRAM | 64 → 128 bytes | The timed effects (steps 08–09) brought the driver to 45 bytes, and the continuous effects need per-channel state: A adds a volume and a slide per channel (53 bytes); arpeggio, portamento and vibrato will need more. |
| 14 (tone portamento) | ROM | 2048 → 3072 bytes | After portamento (step 13) the driver used 1892 bytes. Tone portamento and vibrato do not fit in the remaining 156 bytes, and both are wanted. Saving space by merging the per-channel code paths first was considered, but the gain was uncertain (about 200–300 bytes). |
