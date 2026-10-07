; Golem sound driver. Interface: golem.inc. Behaviour: docs/driver-contract.md.
; Current scope: docs/driver-steps/ (step 4, all four channels).

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
DEF EFFECT_SET_VOLUME EQU $C
DEF WAVE_BYTES EQU 16
DEF NO_WAVE EQU $FF

; Per-channel state, CHANNEL_SIZE bytes each in wChannels.
RSRESET
DEF CHANNEL_INSTRUMENT RB 1 ; 1..15
DEF CHANNEL_PERIOD RB 2 ; Little-endian, period of the last note.
DEF CHANNEL_UNUSED RB 1
DEF CHANNEL_SIZE RB 0
ASSERT CHANNEL_SIZE == 4, "ChannelState and GolemInit assume 4 bytes per channel"

SECTION "Golem state", WRAM0
wSong: ds 2
wTicksPerRow: ds 1 ; Raw header value: 0 means 256.
wTick: ds 1
wRow: ds 1
wOrder: ds 1
wOrderCount: ds 1
wChannel: ds 1 ; Channel being played, 0-based.
wChannels: ds CHANNEL_SIZE * CHANNELS
wLoadedWave: ds 1 ; Wave in wave RAM, or NO_WAVE.

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
	ld a, NO_WAVE
	ld [wLoadedWave], a
	; Every channel starts with instrument 1 and period 0.
	ld hl, wChannels
	ld b, CHANNELS
.channel
	ld a, 1
	ld [hl+], a ; CHANNEL_INSTRUMENT
	xor a
	ld [hl+], a ; CHANNEL_PERIOD
	ld [hl+], a
	ld [hl+], a ; CHANNEL_UNUSED
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
	call z, PlayRow

	; Next tick. Comparing with the raw ticks per row makes 0 mean 256: the tick wraps
	; from 255 to 0.
	ld a, [wTicksPerRow]
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

; Plays the current row of channel wChannel.
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

	ld a, c
	swap a
	and $0F
	jr z, .instrumentDone
	ld d, a
	call ChannelState
	ld [hl], d ; CHANNEL_INSTRUMENT
.instrumentDone
	ld a, [wChannel]
	cp WAVE_CHANNEL
	jr z, PlayWaveCell
	cp NOISE_CHANNEL
	jp z, PlayNoiseCell
	ld a, b
	and a
	jp nz, TriggerPulse

	ld a, c
	and $0F
	cp EFFECT_SET_VOLUME
	ret nz
	; Set volume without a note: NRx2 = xx, then retrigger.
	call PulseNrx1Address
	inc c
	ld a, e
	ldh [c], a ; NRx2
	inc c
	inc c
	call PulseNrx4
	ldh [c], a ; NRx4
	ret

; Plays the current row of the wave channel: b = note, c = instrument << 4 | effect,
; e = effect parameter.
PlayWaveCell:
	ld a, b
	and a
	jr nz, TriggerWave

	ld a, c
	and $0F
	cp EFFECT_SET_VOLUME
	ret nz
	; Set volume without a note: NR32 = (x & 3) << 5, no retrigger.
	ld a, e
	swap a
	and $03
	rrca ; Bits 1-0 to bits 6-5.
	rrca
	rrca
	ldh [rNR32], a
	ret

; Triggers the wave channel with the note in b and its current instrument.
TriggerWave:
	call SetNotePeriod
	ld a, HDR_WAVE_INSTRUMENTS
	call TwoByteInstrument
	push hl
	inc hl
	ld a, [hl] ; Byte 1, bits 3-0: wave index.
	and $0F
	call LoadWave
	pop hl

	ld a, $80 ; DAC on
	ldh [rNR30], a
	ld a, [hl+]
	ldh [rNR31], a
	ld a, [hl]
	ld d, a
	and $60 ; Volume code.
	ldh [rNR32], a
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, [hl+]
	ldh [rNR33], a
	ld a, d
	and $80 ; Length enable, bit 7 of instrument byte 1.
	rrca
	or $80
	or [hl]
	ldh [rNR34], a
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

; Plays the current row of the noise channel: b = note, c = instrument << 4 | effect,
; e = effect parameter.
PlayNoiseCell:
	ld a, b
	and a
	jr nz, TriggerNoise

	ld a, c
	and $0F
	cp EFFECT_SET_VOLUME
	ret nz
	; Set volume without a note: NR42 = xx, then retrigger.
	ld a, e
	ldh [rNR42], a
	ld a, HDR_NOISE_INSTRUMENTS
	call TwoByteInstrument
	ld a, [hl]
	and $40 ; Length enable, already at its NR44 position.
	or $80
	ldh [rNR44], a
	ret

; Triggers the noise channel with the note in b and its current instrument.
TriggerNoise:
	ld a, HDR_NOISE_INSTRUMENTS
	call TwoByteInstrument
	ld a, [hl+]
	ld d, a ; Byte 0: LFSR width, length enable, length timer.
	and $3F
	ldh [rNR41], a
	ld a, [hl]
	ldh [rNR42], a

	ld a, b
	dec a
	ld l, a
	ld h, 0
	push de
	ld de, NoiseNr43
	add hl, de
	pop de
	ld a, d
	and $80 ; 7-bit LFSR: bit 7 to NR43 bit 3.
	rrca
	rrca
	rrca
	rrca
	or [hl]
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

; Triggers pulse channel wChannel with the note in b and its current instrument.
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
	ld a, [hl+]
	ldh [c], a ; NRx1
	inc c
	ld a, [hl]
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
