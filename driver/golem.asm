; Golem sound driver. Interface: golem.inc. Behaviour: docs/driver-contract.md.
; Current scope: docs/driver-steps/ (step 9, all channels, flow, register and timed effects).

INCLUDE "apu.inc"
INCLUDE "golem.inc"

; Song header offsets (docs/song-format.md).
DEF HDR_TICKS_PER_ROW EQU 0
DEF HDR_ORDER_COUNT EQU 1
DEF HDR_ORDER_TABLES EQU 3 ; One pointer per channel.
DEF HDR_PULSE_INSTRUMENTS EQU 11
DEF HDR_WAVE_INSTRUMENTS EQU 13
DEF HDR_NOISE_INSTRUMENTS EQU 15
DEF HDR_WAVES EQU 17

DEF CHANNELS EQU 4
DEF WAVE_CHANNEL EQU 2 ; 0-based
DEF NOISE_CHANNEL EQU 3
DEF ROWS_PER_PATTERN EQU 64
DEF EFFECT_SET_MASTER_VOLUME EQU $5
DEF EFFECT_NOTE_DELAY EQU $7
DEF EFFECT_SET_PANNING EQU $8
DEF EFFECT_CHANGE_TIMBRE EQU $9
DEF EFFECT_POSITION_JUMP EQU $B
DEF EFFECT_SET_VOLUME EQU $C
DEF EFFECT_PATTERN_BREAK EQU $D
DEF EFFECT_NOTE_CUT EQU $E
DEF EFFECT_SET_TEMPO EQU $F
DEF WAVE_BYTES EQU 16
DEF NO_WAVE EQU $FF
DEF NO_CUT EQU 0 ; Tick 0 is never pending: E00 cuts at once.
DEF NO_DELAY EQU 0 ; Tick 0 is never pending: 700 triggers at once.

; wFlow bits: flow effects waiting for the end of the row.
DEF FLOW_JUMP EQU 0 ; B: continue at wJumpOrder.
DEF FLOW_BREAK EQU 1 ; D: continue at row wBreakRow.

; wOverride bits: 9 or C on a row with a note, folded into the trigger.
DEF OVERRIDE_TIMBRE EQU 0 ; 9
DEF OVERRIDE_VOLUME EQU 1 ; C

; Per-channel state, CHANNEL_SIZE bytes each in wChannels.
RSRESET
DEF CHANNEL_INSTRUMENT RB 1 ; 1..15
DEF CHANNEL_PERIOD RB 2 ; Little-endian, period of the last note.
DEF CHANNEL_NOISE RB 1 ; Channel 4: NR43 of the last note, without the width bit.
DEF CHANNEL_SIZE RB 0
ASSERT CHANNEL_SIZE == 4, "ChannelState and GolemInit assume 4 bytes per channel"
ASSERT CHANNEL_NOISE == 3, "TriggerNoise and NoiseTimbre assume CHANNEL_NOISE at offset 3"

SECTION "Golem state", WRAM0
wSong: ds 2
wTicksPerRow: ds 1 ; Tempo: raw header or F value, 0 means 256.
wRowLength: ds 1 ; Tempo when the current row started.
wTick: ds 1
wRow: ds 1
wOrder: ds 1
wOrderCount: ds 1
wFlow: ds 1 ; FLOW_JUMP and FLOW_BREAK flags.
wJumpOrder: ds 1
wBreakRow: ds 1
wChannel: ds 1 ; Channel being played, 0-based.
wEffect: ds 1 ; Effect of the cell being played.
wParam: ds 1 ; Its parameter.
wOverride: ds 1 ; OVERRIDE_TIMBRE and OVERRIDE_VOLUME flags.
wChannels: ds CHANNEL_SIZE * CHANNELS
wLoadedWave: ds 1 ; Wave in wave RAM, or NO_WAVE.
; Timed effects of the current row, per channel. Kept in this order (see the ASSERTs).
wCutTicks: ds CHANNELS ; Tick of a pending E, or NO_CUT.
wDelayTicks: ds CHANNELS ; Tick of a pending 7, or NO_DELAY.
wDelayNotes: ds CHANNELS ; Note of that pending 7.
wTimedPending: ds 1 ; Non-zero if any channel has a pending E or 7 in the current row.

ASSERT wDelayTicks == wCutTicks + CHANNELS, "ClearTimed and PlayTimed assume this layout"
ASSERT wDelayNotes == wDelayTicks + CHANNELS, "PlayCell and PlayTimed assume this layout"

SECTION "Golem driver", ROM0
GolemInit::
	ld a, l
	ld [wSong], a
	ld a, h
	ld [wSong + 1], a
	ld a, [hl] ; HDR_TICKS_PER_ROW
	ld [wTicksPerRow], a
	ld a, HDR_ORDER_COUNT
	call SongHeaderWord
	ld a, [hl]
	ld [wOrderCount], a

	xor a
	ld [wTick], a
	ld [wRow], a
	ld [wOrder], a
	ld [wFlow], a
	ld a, NO_WAVE
	ld [wLoadedWave], a
	call ClearTimed
	; Every channel starts with instrument 1, period 0 and noise value 0.
	ld hl, wChannels
	ld b, CHANNELS
.channel
	ld a, 1
	ld [hl+], a ; CHANNEL_INSTRUMENT
	xor a
	ld [hl+], a ; CHANNEL_PERIOD
	ld [hl+], a
	ld [hl+], a ; CHANNEL_NOISE
	dec b
	jr nz, .channel

	ld a, $80 ; APU on
	ldh [rNR52], a
	ld a, $77 ; Full volume on both outputs
	ldh [rNR50], a
	ld a, $FF ; Every channel on both outputs
	ldh [rNR51], a
	ret

GolemPlay::
	ld a, [wTick]
	and a
	jr nz, .nonRowTick
	; Row tick. The row keeps the tempo it starts with: F only changes the next row.
	ld a, [wTicksPerRow]
	ld [wRowLength], a
	call ClearTimed
	call PlayRow
	jr .tick
.nonRowTick
	call PlayTimed
.tick
	; Next tick. Comparing with the raw row length makes 0 mean 256: the tick wraps
	; from 255 to 0.
	ld a, [wRowLength]
	ld b, a
	ld a, [wTick]
	inc a
	cp b
	jr z, .nextRow
	ld [wTick], a
	ret
.nextRow
	xor a
	ld [wTick], a
	ld a, [wFlow]
	and a
	jr nz, .flow
	ld a, [wRow]
	inc a
	cp ROWS_PER_PATTERN
	jr z, .nextOrder
	ld [wRow], a
	ret
.nextOrder
	xor a
	ld [wRow], a
	ld a, [wOrderCount]
	ld b, a
	ld a, [wOrder]
	inc a
	cp b
	jr c, .storeOrder
	xor a ; Past the last order: loop to order 0.
.storeOrder
	ld [wOrder], a
	ret

.flow
	; B and/or D on the row that just ended. a = wFlow.
	ld b, a
	xor a
	ld [wFlow], a
	; Order: B's target, else the next one. Out of range (also past the last order) is 0.
	ld a, [wOrder]
	inc a
	bit FLOW_JUMP, b
	jr z, .checkOrder
	ld a, [wJumpOrder]
.checkOrder
	ld c, a
	ld a, [wOrderCount]
	ld d, a
	ld a, c
	cp d
	jr c, .setOrder
	xor a
.setOrder
	ld [wOrder], a
	; Row: D's target, else 0. Out of range is 0.
	xor a
	bit FLOW_BREAK, b
	jr z, .setRow
	ld a, [wBreakRow]
	cp ROWS_PER_PATTERN
	jr c, .setRow
	xor a
.setRow
	ld [wRow], a
	ret

; Plays the current row of every channel, in channel order (row tick).
PlayRow:
	xor a
.channel
	ld [wChannel], a
	call PlayCell
	ld a, [wChannel]
	inc a
	cp CHANNELS
	jr nz, .channel
	ret

; Plays the current row of channel wChannel: instrument column, flow effects, trigger,
; then the effect's own writes.
PlayCell:
	; hl = order table of the channel, then the pattern of the current order.
	ld a, [wChannel]
	add a
	add HDR_ORDER_TABLES
	call SongHeaderWord
	ld a, [wOrder]
	ld e, a
	ld d, 0
	add hl, de
	add hl, de
	ld a, [hl+]
	ld h, [hl]
	ld l, a
	; hl = cell of the current row (3 bytes per row).
	ld a, [wRow]
	ld e, a
	add hl, de
	add hl, de
	add hl, de
	; b = note, c = instrument << 4 | effect, e = effect parameter.
	ld a, [hl+]
	ld b, a
	ld a, [hl+]
	ld c, a
	ld e, [hl]
	and $0F
	ld [wEffect], a
	ld a, e
	ld [wParam], a

	ld a, c
	swap a
	and $0F
	jr z, .instrumentDone
	ld d, a
	call ChannelState
	ld [hl], d ; CHANNEL_INSTRUMENT
.instrumentDone
	; Flow effects make no APU writes: record them whatever else the row does. A later
	; channel overwrites an earlier one's target, so the highest channel wins.
	ld a, [wEffect]
	cp EFFECT_POSITION_JUMP
	jr z, .positionJump
	cp EFFECT_PATTERN_BREAK
	jr z, .patternBreak
	cp EFFECT_SET_TEMPO
	jr nz, .override
	ld a, e
	ld [wTicksPerRow], a
	jr .override
.positionJump
	ld a, e
	ld [wJumpOrder], a
	ld hl, wFlow
	set FLOW_JUMP, [hl]
	jr .override
.patternBreak
	ld a, e
	ld [wBreakRow], a
	ld hl, wFlow
	set FLOW_BREAK, [hl]

.override
	; 7 xx with a note: the trigger moves to tick xx of the row, or is dropped past it.
	ld a, b
	and a
	jr z, .noDelay
	ld a, [wEffect]
	cp EFFECT_NOTE_DELAY
	jr nz, .noDelay
	ld a, [wParam]
	and a
	jr z, .noDelay ; 700: an ordinary trigger.
	ld c, a
	ld a, [wRowLength]
	and a
	jr z, .pendingDelay ; 256-tick row: every tick 1-255 is in the row.
	cp c
	ret c ; Row length < xx: dropped.
	ret z ; Row length = xx: dropped.
.pendingDelay
	call CutTickAddress
	ld de, CHANNELS
	add hl, de ; wDelayTicks
	ld [hl], c
	add hl, de ; wDelayNotes
	ld [hl], b
	ld a, 1
	ld [wTimedPending], a
	ret

.noDelay
	; 9 or C on a row with a note change the trigger instead of making their own writes.
	ld d, 0
	ld a, b
	and a
	jr z, .setOverride
	ld a, [wEffect]
	cp EFFECT_CHANGE_TIMBRE
	jr nz, .notTimbre
	ld d, 1 << OVERRIDE_TIMBRE
	jr .setOverride
.notTimbre
	cp EFFECT_SET_VOLUME
	jr nz, .setOverride
	ld d, 1 << OVERRIDE_VOLUME
.setOverride
	ld a, d
	ld [wOverride], a

	ld a, b
	and a
	jp z, ApplyEffect
	call Trigger
	ld a, [wOverride]
	and a
	ret nz ; The effect was folded into the trigger.
	; Fall through: 5 and 8 are written after the trigger.

; Makes the APU writes of the row's effect: 5, 8, and 9 and C without a note. Effects
; without writes (6, B, D, F) and those not specified yet do nothing here.
ApplyEffect:
	ld a, [wEffect]
	cp EFFECT_SET_MASTER_VOLUME
	jr z, .masterVolume
	cp EFFECT_SET_PANNING
	jr z, .panning
	cp EFFECT_CHANGE_TIMBRE
	jr z, .timbre
	cp EFFECT_SET_VOLUME
	jp z, SetVolume
	cp EFFECT_NOTE_CUT
	ret nz
	; E: cut now (E00), or at tick xx of this row if the row is that long.
	ld a, [wParam]
	and a
	jp z, Cut
	ld b, a
	ld a, [wRowLength]
	and a
	jr z, .pendingCut ; 256-tick row: every tick 1-255 is in the row.
	cp b
	ret c ; Row length < xx
	ret z ; Row length = xx
.pendingCut
	call CutTickAddress
	ld [hl], b
	ld a, 1
	ld [wTimedPending], a
	ret
.timbre
	ld a, [wChannel]
	cp WAVE_CHANNEL
	jp z, WaveTimbre
	cp NOISE_CHANNEL
	jp z, NoiseTimbre
	jp PulseTimbre
.masterVolume
	ld a, [wParam]
	ldh [rNR50], a
	ret
.panning
	ld a, [wParam]
	ldh [rNR51], a
	ret

; Silences channel wChannel like C00 (E).
Cut:
	xor a
	ld [wParam], a
	; Fall through.

; C without a note on channel wChannel, with parameter wParam.
SetVolume:
	ld a, [wChannel]
	cp WAVE_CHANNEL
	jp z, WaveSetVolume
	cp NOISE_CHANNEL
	jp z, NoiseSetVolume
	jp PulseSetVolume

; Writes the cuts (E) and delayed triggers (7) due at tick wTick, in channel order
; (non-row ticks). A channel has at most one: a cell holds a single effect.
PlayTimed:
	ld a, [wTimedPending]
	and a
	ret z ; Most ticks: nothing to check.
	xor a
.channel
	ld [wChannel], a
	call CutTickAddress
	ld a, [wTick]
	cp [hl]
	jr nz, .delay
	ld [hl], NO_CUT
	call Cut
	jr .next
.delay
	ld de, CHANNELS
	add hl, de ; wDelayTicks
	cp [hl]
	jr nz, .next
	ld [hl], NO_DELAY
	add hl, de ; wDelayNotes
	ld b, [hl]
	xor a ; The cell's effect is 7: no 9 or C to fold.
	ld [wOverride], a
	call Trigger
.next
	ld a, [wChannel]
	inc a
	cp CHANNELS
	jr nz, .channel
	ret

; Clears every pending cut and delay (row tick). Clobbers af, b, hl.
ClearTimed:
	ASSERT NO_CUT == 0 && NO_DELAY == 0
	ld hl, wCutTicks
	ld b, 2 * CHANNELS ; wCutTicks and wDelayTicks
	xor a
.channel
	ld [hl+], a
	dec b
	jr nz, .channel
	ld [wTimedPending], a
	ret

; Out: hl = pending cut tick of channel wChannel. Clobbers af.
CutTickAddress:
	ld a, [wChannel]
	add LOW(wCutTicks)
	ld l, a
	adc HIGH(wCutTicks)
	sub l
	ld h, a
	ret

; C without a note on a pulse channel: NRx2 = xx, then retrigger.
PulseSetVolume:
	call PulseNrx1Address
	inc c
	ld a, [wParam]
	ldh [c], a ; NRx2
	inc c
	inc c
	call PulseNrx4
	ldh [c], a ; NRx4
	ret

; 9 without a note on a pulse channel: NRx1 = xx.
PulseTimbre:
	call PulseNrx1Address
	ld a, [wParam]
	ldh [c], a ; NRx1
	ret

; C without a note on the wave channel: NR32 = (x & 3) << 5, no retrigger.
WaveSetVolume:
	call WaveVolumeFromParam
	ldh [rNR32], a
	ret

; 9 without a note on the wave channel: load wave y and retrigger, unless it is loaded.
WaveTimbre:
	ld a, [wParam]
	and $0F
	ld hl, wLoadedWave
	cp [hl]
	ret z
	call LoadWave
	ld a, $80 ; DAC on
	ldh [rNR30], a
	call WaveNr34
	ldh [rNR34], a
	ret

; C without a note on the noise channel: NR42 = xx, then retrigger.
NoiseSetVolume:
	ld a, [wParam]
	ldh [rNR42], a
	ld a, HDR_NOISE_INSTRUMENTS
	call TwoByteInstrument
	ld a, [hl]
	and $40 ; Length enable, already at its NR44 position.
	or $80
	ldh [rNR44], a
	ret

; 9 without a note on the noise channel: NR43 = noise value of the last note, with the
; 7-bit LFSR bit when xx is not 0.
NoiseTimbre:
	call ChannelState
	inc hl
	inc hl
	inc hl ; CHANNEL_NOISE
	ld a, [wParam]
	and a
	ld a, [hl]
	jr z, .write
	or $08
.write
	ldh [rNR43], a
	ret

; Triggers channel wChannel with the note in b, its current instrument and wOverride.
Trigger:
	ld a, [wChannel]
	cp WAVE_CHANNEL
	jp z, TriggerWave
	cp NOISE_CHANNEL
	jp z, TriggerNoise
	jp TriggerPulse

; Triggers the wave channel with the note in b.
TriggerWave:
	call SetNotePeriod
	ld a, HDR_WAVE_INSTRUMENTS
	call TwoByteInstrument
	push hl
	inc hl
	ld a, [hl] ; Byte 1, bits 3-0: wave index, unless 9 gives it.
	and $0F
	ld e, a
	ld a, [wOverride]
	bit OVERRIDE_TIMBRE, a
	ld a, e
	jr z, .loadWave
	ld a, [wParam]
	and $0F
.loadWave
	call LoadWave
	pop hl

	ld a, $80 ; DAC on
	ldh [rNR30], a
	ld a, [hl+]
	ldh [rNR31], a
	ld a, [hl]
	ld d, a
	and $60 ; Volume code, unless C gives it.
	ld e, a
	ld a, [wOverride]
	bit OVERRIDE_VOLUME, a
	ld a, e
	jr z, .volume
	call WaveVolumeFromParam
.volume
	ldh [rNR32], a
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, [hl]
	ldh [rNR33], a
	call WaveNr34
	ldh [rNR34], a
	ret

; Out: a = NR34 for a trigger: $80 | length enable << 6 | period bits 10-8. Clobbers de, hl.
WaveNr34:
	ld a, HDR_WAVE_INSTRUMENTS
	call TwoByteInstrument
	inc hl
	ld a, [hl]
	and $80 ; Length enable, bit 7 of instrument byte 1.
	rrca
	or $80
	ld d, a
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	inc hl
	ld a, [hl]
	or d
	ret

; Out: a = NR32 volume code from the x nibble of wParam: (x & 3) << 5. Clobbers f.
WaveVolumeFromParam:
	ld a, [wParam]
	swap a
	and $03
	rrca ; Bits 1-0 to bits 6-5.
	rrca
	rrca
	ret

; Loads wave a (0-15) into wave RAM, unless it is already there. Clobbers af, bc, de, hl.
LoadWave:
	ld hl, wLoadedWave
	cp [hl]
	ret z
	ld [hl], a
	swap a ; WAVE_BYTES * index
	ld e, a
	ld d, 0
	push de
	ld a, HDR_WAVES
	call SongHeaderWord
	pop de
	add hl, de

	xor a ; DAC off while writing wave RAM.
	ldh [rNR30], a
	ld c, LOW(_WAVE_RAM)
	ld b, WAVE_BYTES
.byte
	ld a, [hl+]
	ldh [c], a
	inc c
	dec b
	jr nz, .byte
	ret

; Triggers the noise channel with the note in b.
TriggerNoise:
	ld a, HDR_NOISE_INSTRUMENTS
	call TwoByteInstrument
	ld a, [hl+]
	ld d, a ; Byte 0: LFSR width, length enable, length timer.
	and $3F
	ldh [rNR41], a
	ld a, [wOverride]
	ld e, a
	ld a, [hl] ; Envelope, unless C gives it.
	bit OVERRIDE_VOLUME, e
	jr z, .envelope
	ld a, [wParam]
.envelope
	ldh [rNR42], a

	; c = noise value of the note, kept for 9 without a note.
	ld a, b
	dec a
	add LOW(NoiseNr43)
	ld l, a
	adc HIGH(NoiseNr43)
	sub l
	ld h, a
	ld c, [hl]
	call ChannelState
	inc hl
	inc hl
	inc hl ; CHANNEL_NOISE
	ld [hl], c

	; LFSR width: from the instrument (bit 7), unless 9 gives it (xx != 0).
	bit OVERRIDE_TIMBRE, e
	jr nz, .timbreWidth
	ld a, d
	and $80
	jr .width
.timbreWidth
	ld a, [wParam]
	and a
	jr z, .width
	ld a, $80
.width
	rrca ; Bit 7 to NR43 bit 3.
	rrca
	rrca
	rrca
	or c
	ldh [rNR43], a

	ld a, d
	and $40 ; Length enable, already at its NR44 position.
	or $80
	ldh [rNR44], a
	ret

; In: a = header offset of a table of 2-byte instruments (wave or noise).
; Out: hl = current instrument of channel wChannel in that table. Clobbers af, de.
TwoByteInstrument:
	push af
	call ChannelState
	ld a, [hl] ; CHANNEL_INSTRUMENT
	dec a
	add a
	ld e, a
	ld d, 0
	pop af
	push de
	call SongHeaderWord
	pop de
	add hl, de
	ret

; Stores the period of note b in the state of channel wChannel. Clobbers af, de, hl.
SetNotePeriod:
	ld a, b
	dec a
	ld l, a
	ld h, 0
	add hl, hl
	ld de, NotePeriods
	add hl, de
	ld a, [hl+]
	ld d, [hl]
	ld e, a
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, e
	ld [hl+], a
	ld [hl], d
	ret

; Triggers pulse channel wChannel with the note in b.
TriggerPulse:
	call SetNotePeriod
	call PulseInstrument
	ld a, [wChannel]
	and a
	jr nz, .sweepDone
	ld a, [hl]
	and $7F ; Without the length enable bit.
	ldh [rNR10], a
.sweepDone
	inc hl
	call PulseNrx1Address
	ld a, [wOverride]
	ld e, a
	ld a, [hl+] ; Duty and length, unless 9 gives them.
	bit OVERRIDE_TIMBRE, e
	jr z, .nrx1
	ld a, [wParam]
.nrx1
	ldh [c], a ; NRx1
	inc c
	ld a, [hl] ; Envelope, unless C gives it.
	bit OVERRIDE_VOLUME, e
	jr z, .nrx2
	ld a, [wParam]
.nrx2
	ldh [c], a ; NRx2
	inc c
	call ChannelState
	inc hl ; CHANNEL_PERIOD, low byte
	ld a, [hl]
	ldh [c], a ; NRx3
	inc c
	call PulseNrx4
	ldh [c], a ; NRx4
	ret

; Out: a = NRx4 of pulse channel wChannel for a trigger:
; $80 | length enable << 6 | period bits 10-8. Clobbers de, hl.
PulseNrx4:
	call PulseInstrument
	ld a, [hl]
	and $80 ; Length enable, bit 7 of instrument byte 0.
	rrca
	or $80
	ld d, a
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	inc hl
	ld a, [hl]
	or d
	ret

; Out: c = low byte of NRx1 of pulse channel wChannel ($11 or $16). Clobbers af.
PulseNrx1Address:
	ld a, [wChannel]
	ld c, a
	add a
	add a
	add c ; 5 registers per channel.
	add LOW(rNR11)
	ld c, a
	ret

; Out: hl = current pulse instrument of channel wChannel (3 bytes). Clobbers af, de.
PulseInstrument:
	call ChannelState
	ld a, [hl] ; CHANNEL_INSTRUMENT
	dec a
	ld e, a
	ld d, 0
	push de
	ld a, HDR_PULSE_INSTRUMENTS
	call SongHeaderWord
	pop de
	add hl, de
	add hl, de
	add hl, de
	ret

; Out: hl = state of channel wChannel. Clobbers af.
ChannelState:
	ld a, [wChannel]
	add a
	add a ; CHANNEL_SIZE
	add LOW(wChannels)
	ld l, a
	adc HIGH(wChannels)
	sub l
	ld h, a
	ret

; In: a = offset of a pointer in the song header. Out: hl = that pointer. Clobbers af, de.
SongHeaderWord:
	ld e, a
	ld d, 0
	ld a, [wSong]
	ld l, a
	ld a, [wSong + 1]
	ld h, a
	add hl, de
	ld a, [hl+]
	ld h, [hl]
	ld l, a
	ret

INCLUDE "notes.inc"
