; Golem sound driver. Interface: golem.inc. Behaviour: docs/driver-contract.md.
; Current scope: docs/driver-steps/ (step 1, channel 1 only).

INCLUDE "apu.inc"
INCLUDE "golem.inc"

; Song header offsets (docs/song-format.md).
DEF HDR_TICKS_PER_ROW EQU 0
DEF HDR_ORDER_COUNT EQU 1
DEF HDR_ORDER_TABLE_1 EQU 3
DEF HDR_PULSE_INSTRUMENTS EQU 11

DEF ROWS_PER_PATTERN EQU 64
DEF EFFECT_SET_VOLUME EQU $C

SECTION "Golem state", WRAM0
wSong: ds 2
wTicksPerRow: ds 1 ; Raw header value: 0 means 256.
wTick: ds 1
wRow: ds 1
wOrder: ds 1
wOrderCount: ds 1
wCh1Instrument: ds 1 ; 1..15
wCh1Period: ds 2 ; Little-endian, period of the last note.

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
	ld [wCh1Period], a
	ld [wCh1Period + 1], a
	inc a
	ld [wCh1Instrument], a

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

; Plays the current row of channel 1 (row tick).
PlayRow:
	; hl = order table of channel 1, then the pattern of the current order.
	ld a, HDR_ORDER_TABLE_1
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
	ld [wCh1Instrument], a
.instrumentDone
	ld a, b
	and a
	jr z, .noNote
	jp TriggerChannel1

.noNote
	ld a, c
	and $0F
	cp EFFECT_SET_VOLUME
	ret nz
	; Set volume without a note: NR12 = xx, then retrigger.
	ld a, e
	ldh [rNR12], a
	call Channel1Nr14
	ldh [rNR14], a
	ret

; Triggers channel 1 with the note in b and the current instrument.
TriggerChannel1:
	ld a, b
	dec a
	ld l, a
	ld h, 0
	add hl, hl
	ld de, NotePeriods
	add hl, de
	ld a, [hl+]
	ld [wCh1Period], a
	ld a, [hl]
	ld [wCh1Period + 1], a

	call Channel1Instrument
	ld a, [hl+]
	and $7F ; Without the length enable bit.
	ldh [rNR10], a
	ld a, [hl+]
	ldh [rNR11], a
	ld a, [hl]
	ldh [rNR12], a
	ld a, [wCh1Period]
	ldh [rNR13], a
	call Channel1Nr14
	ldh [rNR14], a
	ret

; Out: a = NR14 for a trigger: $80 | length enable << 6 | period bits 10-8.
; Clobbers de, hl.
Channel1Nr14:
	call Channel1Instrument
	ld a, [hl]
	and $80 ; Length enable, bit 7 of instrument byte 0.
	rrca
	or $80
	ld d, a
	ld a, [wCh1Period + 1]
	or d
	ret

; Out: hl = current pulse instrument of channel 1 (3 bytes). Clobbers af, de.
Channel1Instrument:
	ld a, HDR_PULSE_INSTRUMENTS
	call SongHeaderWord
	ld a, [wCh1Instrument]
	dec a
	ld e, a
	ld d, 0
	add hl, de
	add hl, de
	add hl, de
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
