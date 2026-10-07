; Golem sound driver. Interface: golem.inc. Behaviour: docs/driver-contract.md.
; Current scope: docs/driver-steps/ (step 15: every effect of the song format, 0-F).

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
DEF HDR_CACHED_SIZE EQU HDR_WAVES + 2 - HDR_ORDER_TABLES ; Pointers kept in wHeaderCache.

DEF CHANNELS EQU 4
DEF WAVE_CHANNEL EQU 2 ; 0-based
DEF NOISE_CHANNEL EQU 3
DEF ROWS_PER_PATTERN EQU 64
DEF LAST_NOTE EQU 72 ; B-7
DEF EFFECT_ARPEGGIO EQU $0
DEF EFFECT_PORTAMENTO_UP EQU $1
DEF EFFECT_PORTAMENTO_DOWN EQU $2
DEF EFFECT_TONE_PORTAMENTO EQU $3
DEF EFFECT_VIBRATO EQU $4
DEF EFFECT_SET_MASTER_VOLUME EQU $5
DEF EFFECT_NOTE_DELAY EQU $7
DEF EFFECT_SET_PANNING EQU $8
DEF EFFECT_CHANGE_TIMBRE EQU $9
DEF EFFECT_VOLUME_SLIDE EQU $A
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
wHeaderCache: ds HDR_CACHED_SIZE ; Song header from HDR_ORDER_TABLES on: order tables, instruments, waves.
wPatterns: ds 2 * CHANNELS ; Pattern of each channel in the current order.
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
wSlides: ds CHANNELS ; Parameter of the row's A, or 0 (no slide, like A00).
wArpeggios: ds CHANNELS ; Parameter of the row's 0, or 0 (no arpeggio).
wPortas: ds CHANNELS ; Step of the row's 1 or 2, or 0 (no portamento).
wTonePortas: ds CHANNELS ; Step of the row's 3, or 0 (no tone portamento).
wVibratos: ds CHANNELS ; Parameter of the row's 4, or 0 (no vibrato).
wVibratoCounts: ds CHANNELS ; Bit 7: the - y phase; bits 0-6: its ticks left; 0: first step.
wDelayNotes: ds CHANNELS ; Note of a pending 7.
wPortaDown: ds CHANNELS ; Non-zero for 2 (down); read only while wPortas is set.
wTimedPending: ds 1 ; Non-zero if any channel has a pending E, 7, A, 0, 1, 2, 3 or 4 in the row.
wPhase: ds 1 ; wTick % 3, for the arpeggio.
wPitchMoved: ds 1 ; Non-zero once an arpeggio step moved a pitch: the next row tick restores.
; Per channel, kept from row to row; cleared together by GolemInit.
wVolumes: ds CHANNELS ; Volume (0-15) of channels 1, 2 and 4, for A.
wNotes: ds CHANNELS ; Last note triggered on channels 1-3, or 0.
wPitches: ds 2 * CHANNELS ; Period in NRx3/NRx4 of channels 1-3 (little-endian).
wTargets: ds 2 * CHANNELS ; Period a 3 slides toward, or 0 (none), until the next trigger.

ASSERT wDelayTicks == wCutTicks + CHANNELS, "ClearTimed and PlayTimed assume this layout"
ASSERT wSlides == wDelayTicks + CHANNELS, "ClearTimed, PlayTimed and ApplyEffect assume this layout"
ASSERT wArpeggios == wSlides + CHANNELS, "ClearTimed and PlayTimed assume this layout"
ASSERT wPortas == wArpeggios + CHANNELS, "ClearTimed and PlayTimed assume this layout"
ASSERT wTonePortas == wPortas + CHANNELS, "ClearTimed and PlayTimed assume this layout"
ASSERT wVibratos == wTonePortas + CHANNELS, "ClearTimed and PlayTimed assume this layout"
ASSERT wVibratoCounts == wVibratos + CHANNELS, "ClearTimed and VibratoStep assume this layout"
ASSERT wNotes == wVolumes + CHANNELS && wPitches == wNotes + CHANNELS, "GolemInit assumes this layout"
ASSERT wTargets == wPitches + 2 * CHANNELS, "GolemInit assumes this layout"

SECTION "Golem driver", ROM0
GolemInit::
	ld a, [hl+] ; HDR_TICKS_PER_ROW
	ld [wTicksPerRow], a
	ASSERT HDR_ORDER_COUNT == HDR_TICKS_PER_ROW + 1
	ld a, [hl+]
	ld e, a
	ld a, [hl+]
	ld d, a
	ld a, [de]
	ld [wOrderCount], a
	; Keep the header's pointers in WRAM: SongHeaderWord reads them from there.
	ASSERT HDR_ORDER_TABLES == HDR_ORDER_COUNT + 2
	ld de, wHeaderCache
	ld b, HDR_CACHED_SIZE
.header
	ld a, [hl+]
	ld [de], a
	inc de
	dec b
	jr nz, .header

	xor a
	ld [wTick], a
	ld [wRow], a
	ld [wOrder], a
	ld [wFlow], a
	call LoadPatterns
	ld a, NO_WAVE
	ld [wLoadedWave], a
	call ClearTimed
	ld [wPhase], a ; a = 0 after ClearTimed.
	ld [wPitchMoved], a
	ld hl, wVolumes
	ld b, 6 * CHANNELS ; wVolumes, wNotes, wPitches and wTargets
.channelTables
	ld [hl+], a
	dec b
	jr nz, .channelTables
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
	ld a, [wPhase]
	inc a
	cp 3
	jr c, .setPhase
	xor a
.setPhase
	ld [wPhase], a
	ret
.nextRow
	xor a
	ld [wTick], a
	ld [wPhase], a
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
	jp LoadPatterns

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
	jp LoadPatterns

; Loads wPatterns from the order tables, for the order in wOrder. Clobbers all registers.
LoadPatterns:
	ld a, [wOrder]
	ld c, a
	ld b, 0
	sla c
	rl b ; bc = 2 * order: offset in an order table.
	ld de, wPatterns
	ld hl, wHeaderCache ; The order tables come first.
	ld a, CHANNELS
.channel
	push af
	push hl
	ld a, [hl+]
	ld h, [hl]
	ld l, a
	add hl, bc
	ld a, [hl+]
	ld [de], a
	inc de
	ld a, [hl]
	ld [de], a
	inc de
	pop hl
	inc hl
	inc hl
	pop af
	dec a
	jr nz, .channel
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
	; Every pitch is back on its note now: restored, or set by a trigger.
	xor a
	ld [wPitchMoved], a
	ret

; Plays the current row of channel wChannel: instrument column, flow effects, trigger,
; then the effect's own writes.
PlayCell:
	; hl = the channel's pattern in the current order, then the cell of the current row.
	ld a, [wChannel]
	add a
	add LOW(wPatterns)
	ld l, a
	adc HIGH(wPatterns)
	sub l
	ld h, a
	ld a, [hl+]
	ld h, [hl]
	ld l, a
	ld a, [wRow]
	ld e, a
	ld d, 0
	add hl, de
	add hl, de
	add hl, de
	; b = note, c = instrument << 4 | effect, e = effect parameter.
	ld a, [hl+]
	ld b, a
	ld a, [hl+]
	ld c, a
	ld e, [hl]
	ld a, [wPitchMoved]
	and a
	call nz, RestorePitch
	; An empty cell (no note, no instrument, effect 0 with $00) does nothing more.
	ld a, c
	or b
	or e
	ret z
	ld a, c
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
	; 3 with a note, on a channel that already plays one: no trigger. The note becomes the
	; channel's note, and its period the target the period slides toward.
	ld a, b
	and a
	jr z, .noTarget
	ld a, [wEffect]
	cp EFFECT_TONE_PORTAMENTO
	jr nz, .noTarget
	ld a, [wChannel]
	cp NOISE_CHANNEL
	jr z, .noTarget
	ld hl, wNotes
	call ChannelEntry
	ld a, [hl]
	and a
	jr z, .noTarget ; The channel's first note: an ordinary trigger.
	ld [hl], b
	ld a, b
	dec a
	ld l, a
	ld h, 0
	add hl, hl
	ld de, NotePeriods
	add hl, de
	ld a, [hl+]
	ld e, a
	ld d, [hl]
	ld hl, wTargets
	call ChannelWord
	ld a, e
	ld [hl+], a
	ld [hl], d
	jp ApplyEffect ; Records the step (3 is the cell's effect).

.noTarget
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
	ld de, wDelayNotes - wDelayTicks
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
	ASSERT EFFECT_ARPEGGIO == 0
	and a
	jp z, .arpeggio
	cp EFFECT_PORTAMENTO_UP
	jp z, .portamento
	cp EFFECT_PORTAMENTO_DOWN
	jp z, .portamento
	cp EFFECT_TONE_PORTAMENTO
	jp z, .tonePortamento
	cp EFFECT_VIBRATO
	jp z, .vibrato
	cp EFFECT_SET_MASTER_VOLUME
	jp z, .masterVolume
	cp EFFECT_SET_PANNING
	jp z, .panning
	cp EFFECT_CHANGE_TIMBRE
	jp z, .timbre
	cp EFFECT_SET_VOLUME
	jp z, SetVolume
	cp EFFECT_VOLUME_SLIDE
	jp z, .volumeSlide
	cp EFFECT_NOTE_CUT
	ret nz
	; E: cut now (E00), or at tick xx of this row if the row is that long.
	ld a, [wParam]
	and a
	jp z, Cut
	ld b, a
	ld a, [wRowLength]
	and a
	jp z, .pendingCut ; 256-tick row: every tick 1-255 is in the row.
	cp b
	ret c ; Row length < xx
	ret z ; Row length = xx
.pendingCut
	call CutTickAddress
	ld [hl], b
	ld a, 1
	ld [wTimedPending], a
	ret
.arpeggio
	; 0: arpeggio steps on the non-row ticks of this row. Nothing on the noise channel.
	ld a, [wChannel]
	cp NOISE_CHANNEL
	ret z
	ld hl, wArpeggios
	call ChannelEntry
	ld a, [wParam]
	ld [hl], a
	and a
	ret z ; Effect 0 with $00: no arpeggio.
	ld a, 1
	ld [wTimedPending], a
	ret
.vibrato
	; 4: vibrato steps on the non-row ticks of this row. Nothing on the noise channel.
	ld a, [wChannel]
	cp NOISE_CHANNEL
	ret z
	ld hl, wVibratos
	call ChannelEntry
	ld a, [wParam]
	ld [hl], a
	and a
	ret z ; 400: no vibrato.
	ld a, 1
	ld [wTimedPending], a
	ret
.tonePortamento
	; 3: tone portamento steps on the non-row ticks of this row. Nothing on the noise channel.
	ld a, [wChannel]
	cp NOISE_CHANNEL
	ret z
	ld hl, wTonePortas
	call ChannelEntry
	ld a, [wParam]
	ld [hl], a
	and a
	ret z ; 300: no slide.
	ld a, 1
	ld [wTimedPending], a
	ret
.portamento
	; 1 or 2: portamento steps on the non-row ticks of this row. Nothing on noise. a = effect.
	ld b, a
	ld a, [wChannel]
	cp NOISE_CHANNEL
	ret z
	ld hl, wPortas
	call ChannelEntry
	ld a, [wParam]
	ld [hl], a
	ld c, a
	ld de, wPortaDown - wPortas
	add hl, de
	ld a, b
	sub EFFECT_PORTAMENTO_UP ; 0 for up, 1 for down.
	ld [hl], a
	ld a, c
	and a
	ret z ; 00: no portamento.
	ld a, 1
	ld [wTimedPending], a
	ret
.volumeSlide
	; A: a slide step on each non-row tick of this row. Nothing on the wave channel.
	ld a, [wChannel]
	cp WAVE_CHANNEL
	ret z
	call CutTickAddress
	ld de, 2 * CHANNELS
	add hl, de ; wSlides
	ld a, [wParam]
	ld [hl], a
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

; Writes the cuts (E), delayed triggers (7), slide steps (A), arpeggio steps (0),
; portamento steps (1, 2, 3) and vibrato steps (4) due at tick wTick, in channel order
; (non-row ticks). A channel has at most one: a cell holds a
; single effect.
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
	jr nz, .slide
	ld [hl], NO_DELAY
	ld de, wDelayNotes - wDelayTicks
	add hl, de ; wDelayNotes
	ld b, [hl]
	xor a ; The cell's effect is 7: no 9 or C to fold.
	ld [wOverride], a
	call Trigger
	jr .next
.slide
	add hl, de ; wSlides
	ld a, [hl]
	and a
	jr z, .arpeggio
	call SlideVolume
	jr .next
.arpeggio
	add hl, de ; wArpeggios
	ld a, [hl]
	and a
	jr z, .portamento
	call ArpeggioStep
	jr .next
.portamento
	add hl, de ; wPortas
	ld a, [hl]
	and a
	jr z, .tonePortamento
	call PortaStep
	jr .next
.tonePortamento
	add hl, de ; wTonePortas
	ld a, [hl]
	and a
	jr z, .vibrato
	call TonePortaStep
	jr .next
.vibrato
	add hl, de ; wVibratos
	ld a, [hl]
	and a
	call nz, VibratoStep
.next
	ld a, [wChannel]
	inc a
	cp CHANNELS
	jr nz, .channel
	ret

; Clears every per-row effect table, from wCutTicks to wVibratoCounts (row tick).
; Clobbers af, b, hl.
ClearTimed:
	ASSERT NO_CUT == 0 && NO_DELAY == 0
	ld hl, wCutTicks
	ld b, 8 * CHANNELS ; From wCutTicks to wVibratoCounts
	xor a
.channel
	ld [hl+], a
	dec b
	jr nz, .channel
	ld [wTimedPending], a
	ret

; One non-row tick of A on channel wChannel (1, 2 or 4): volume up by x, or down by y,
; clamped to 0-15. If it changed: NRx2 = volume << 4 (envelope pace 0), then retrigger,
; i.e. the writes of C without a note.
SlideVolume:
	call CutTickAddress
	ld de, 2 * CHANNELS
	add hl, de ; wSlides
	ld b, [hl]
	call VolumeAddress
	ld a, b
	swap a
	and $0F ; x
	jr z, .down
	add [hl]
	cp 16
	jr c, .set
	ld a, 15
	jr .set
.down
	ld a, b
	and $0F ; y
	ld c, a
	ld a, [hl]
	sub c
	jr nc, .set
	xor a
.set
	cp [hl]
	ret z ; Unchanged: nothing to write.
	swap a
	ld [wParam], a
	jp SetVolume ; Stores the volume too.

; In: a = an NRx2 value written for channel wChannel. Keeps its volume for A.
; Clobbers af, hl.
StoreVolume:
	swap a
	and $0F
	push af
	call VolumeAddress
	pop af
	ld [hl], a
	ret

; Out: hl = volume of channel wChannel. Clobbers af.
VolumeAddress:
	ld a, [wChannel]
	add LOW(wVolumes)
	ld l, a
	adc HIGH(wVolumes)
	sub l
	ld h, a
	ret

; One non-row tick of 0 xy on channel wChannel (1-3): the channel's period when wPhase is
; 0, the last note + x or + y (clamped to B-7) when it is 1 or 2. Writes the pitch when it
; changes. Clobbers all.
ArpeggioStep:
	ld hl, wArpeggios
	call ChannelEntry
	ld c, [hl]
	ld hl, wNotes
	call ChannelEntry
	ld a, [hl]
	and a
	ret z ; No note yet.
	ld b, a
	ld a, [wPhase]
	and a
	jr nz, .noteStep
	call ChannelPeriod ; Phase 0: the channel's period.
	jr .compare
.noteStep
	dec a
	ld a, c
	jr nz, .y ; Phase 2: y.
	swap a ; Phase 1: x.
.y
	and $0F
	add b
	cp LAST_NOTE + 1
	jr c, .inRange
	ld a, LAST_NOTE
.inRange
	dec a
	ld l, a
	ld h, 0
	add hl, hl
	ld de, NotePeriods
	add hl, de
	ld a, [hl+]
	ld e, a
	ld d, [hl]
.compare
	ld hl, wPitches
	call ChannelWord
	ld a, [hl+]
	cp e
	jr nz, .write
	ld a, [hl]
	cp d
	ret z ; Same pitch: nothing to write.
.write
	ld a, 1
	ld [wPitchMoved], a
	jp WritePitch

; One non-row tick of 1 or 2 on channel wChannel (1-3): the channel's period + or - the
; step, clamped to the note table, becomes its period and pitch. Clobbers all.
PortaStep:
	ld hl, wNotes
	call ChannelEntry
	ld a, [hl]
	and a
	ret z ; No note yet.
	ld hl, wPortas
	call ChannelEntry
	ld c, [hl] ; Step
	ld de, wPortaDown - wPortas
	add hl, de
	ld b, [hl] ; Direction: non-zero for down.
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, [hl+]
	ld e, a
	ld d, [hl] ; de = period
	dec hl
	ld a, b
	and a
	jr nz, .down
	call AddPeriodClamped
	jr .store
.down
	call SubPeriodClamped
.store
	ld a, [hl+] ; hl = CHANNEL_PERIOD
	cp e
	jr nz, .changed
	ld a, [hl]
	cp d
	ret z ; Clamped: nothing changes.
.changed
	ld [hl], d
	dec hl
	ld [hl], e
	jp WritePitch

; One non-row tick of 3 on channel wChannel (1-3): the channel's period moves toward its
; target by the step, without passing it, and becomes its period and pitch. Clobbers all.
TonePortaStep:
	ld hl, wTargets
	call ChannelWord
	ld a, [hl+]
	ld c, a
	ld b, [hl] ; bc = target
	or b
	ret z ; No target.
	ld hl, wTonePortas
	call ChannelEntry
	ld a, [hl]
	ld [wParam], a ; The step; wParam is free on non-row ticks.
	call ChannelPeriod ; de = period
	ld a, d
	cp b
	jr nz, .compared
	ld a, e
	cp c
	ret z ; Already at the target.
.compared
	jr c, .up ; Period below the target.
	ld a, [wParam] ; Down: period - step, not below the target.
	ld l, a
	ld a, e
	sub l
	ld e, a
	jr nc, .checkDown
	dec d
.checkDown
	bit 7, d
	jr nz, .reached ; Went below 0.
	ld a, d
	cp b
	jr c, .reached
	jr nz, .store
	ld a, e
	cp c
	jr c, .reached
	jr .store
.up
	ld a, [wParam] ; Up: period + step, not above the target.
	add e
	ld e, a
	jr nc, .checkUp
	inc d
.checkUp
	ld a, b
	cp d
	jr c, .reached
	jr nz, .store
	ld a, c
	cp e
	jr nc, .store
.reached
	ld d, b
	ld e, c
.store
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, e
	ld [hl+], a
	ld [hl], d
	jp WritePitch

; One non-row tick of 4 xy on channel wChannel (1-3): the channel's period + y for x
; ticks, then - y for x ticks, and so on (x = 0 counts as 1), clamped to the note table.
; Writes the pitch (not the period) when it changes. Clobbers all.
VibratoStep:
	ld hl, wNotes
	call ChannelEntry
	ld a, [hl]
	and a
	ret z ; No note yet.
	ld hl, wVibratos
	call ChannelEntry
	ld c, [hl] ; c = xy
	ld de, wVibratoCounts - wVibratos
	add hl, de
	ld a, [hl]
	and a
	jr nz, .counting
	call VibratoSpeed ; First step of the row: + y, x ticks left.
.counting
	ld b, a ; b = this tick's phase (bit 7) and ticks left
	and $7F
	dec a
	jr nz, .sameNext
	call VibratoSpeed ; Phase over: the other one, x ticks.
	ld e, a
	ld a, b
	and $80
	xor $80
	or e
	jr .setCount
.sameNext
	ld e, a
	ld a, b
	and $80
	or e
.setCount
	ld [hl], a
	call ChannelPeriod ; de = period
	ld a, c
	and $0F
	ld c, a ; c = y
	bit 7, b
	jr nz, .down
	call AddPeriodClamped
	jr .compare
.down
	call SubPeriodClamped
.compare
	ld hl, wPitches
	call ChannelWord
	ld a, [hl+]
	cp e
	jr nz, .write
	ld a, [hl]
	cp d
	ret z ; Same pitch: nothing to write.
.write
	ld a, 1
	ld [wPitchMoved], a
	jp WritePitch

; In: c = vibrato parameter xy. Out: a = x, or 1 if x is 0. Keeps the other registers.
VibratoSpeed:
	ld a, c
	swap a
	and $0F
	ret nz
	inc a
	ret

; de = min(de + c, NOTE_PERIOD_LAST). Clobbers af.
AddPeriodClamped:
	ld a, e
	add c
	ld e, a
	jr nc, .check
	inc d
.check
	ld a, d
	cp HIGH(NOTE_PERIOD_LAST)
	ret c
	jr nz, .clamp
	ld a, e
	cp LOW(NOTE_PERIOD_LAST) + 1
	ret c
.clamp
	ld de, NOTE_PERIOD_LAST
	ret

; de = max(de - c, NOTE_PERIOD_FIRST); de is at most $7FF. Clobbers af.
SubPeriodClamped:
	ld a, e
	sub c
	ld e, a
	jr nc, .check
	dec d
.check
	bit 7, d
	jr nz, .clamp ; Went below 0.
	ld a, d
	and a
	ret nz ; At least 256.
	ASSERT HIGH(NOTE_PERIOD_FIRST) == 0
	ld a, e
	cp LOW(NOTE_PERIOD_FIRST)
	ret nc
.clamp
	ld de, NOTE_PERIOD_FIRST
	ret

; Row tick, before channel wChannel's cell (b = note, c = instrument << 4 | effect, e =
; effect parameter): if an arpeggio or a vibrato left the pitch off the channel's period,
; writes the period back, unless the cell triggers a note on this tick. Keeps bc and de.
RestorePitch:
	ld a, [wChannel]
	cp NOISE_CHANNEL
	ret z
	; A note triggered on this tick writes its own pitch. A note delayed by 7, or a 3 target
	; on a channel that already plays a note, does not.
	ld a, b
	and a
	jr z, .check
	ld a, c
	and $0F
	cp EFFECT_TONE_PORTAMENTO
	jr z, .tonePortamento
	cp EFFECT_NOTE_DELAY
	ret nz
	ld a, e
	and a
	ret z
	jr .check
.tonePortamento
	ld hl, wNotes
	call ChannelEntry
	ld a, [hl]
	and a
	ret z ; The channel's first note: it triggers.
.check
	push bc
	push de
	call ChannelPeriod
	ld hl, wPitches
	call ChannelWord
	ld a, [hl+]
	cp e
	jr nz, .restore
	ld a, [hl]
	cp d
	jr z, .done
.restore
	call WritePitch
.done
	pop de
	pop bc
	ret

; Out: de = period of channel wChannel. Clobbers af, hl.
ChannelPeriod:
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, [hl+]
	ld e, a
	ld d, [hl]
	ret

; Writes period de to NRx3/NRx4 of channel wChannel (1-3) without the trigger bit (with the
; instrument's length bit), and records it as the channel's pitch. Clobbers all.
WritePitch:
	ld hl, wPitches
	call ChannelWord
	ld a, e
	ld [hl+], a
	ld [hl], d
	push de
	ld a, [wChannel]
	cp WAVE_CHANNEL
	jr z, .wave
	call PulseInstrument
	ld a, [hl]
	and $80 ; Length enable, bit 7 of instrument byte 0.
	rrca
	pop de
	or d
	ld d, a
	call PulseNrx1Address
	inc c
	inc c
	ld a, e
	ldh [c], a ; NRx3
	inc c
	ld a, d
	ldh [c], a ; NRx4, no trigger bit
	ret
.wave
	ld a, HDR_WAVE_INSTRUMENTS
	call TwoByteInstrument
	inc hl
	ld a, [hl]
	and $80 ; Length enable, bit 7 of instrument byte 1.
	rrca
	pop de
	or d
	ld d, a
	ld a, e
	ldh [rNR33], a
	ld a, d
	ldh [rNR34], a ; No trigger bit
	ret

; In: hl = a table of two bytes per channel. Out: hl = the entry of channel wChannel.
; Clobbers af.
ChannelWord:
	ld a, [wChannel]
	add a
	add l
	ld l, a
	adc h
	sub l
	ld h, a
	ret

; In: hl = a table of one byte per channel. Out: hl = the entry of channel wChannel.
; Clobbers af.
ChannelEntry:
	ld a, [wChannel]
	add l
	ld l, a
	adc h
	sub l
	ld h, a
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
	call StoreVolume
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
	call StoreVolume
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
	ld a, d
	and $80 ; Length enable, bit 7 of instrument byte 1.
	rrca
	or $80
	ld d, a ; d = NR34 without the period bits.
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, [hl+]
	ldh [rNR33], a
	ld a, [hl]
	or d
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
	call StoreVolume

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

; Stores the period of note b in the state of channel wChannel, b as its last note and the
; period as its pitch, and clears its target (channels 1-3). Clobbers af, de, hl.
SetNotePeriod:
	ld hl, wNotes
	call ChannelEntry
	ld [hl], b
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
	ld hl, wPitches
	call ChannelWord
	ld a, e
	ld [hl+], a
	ld [hl], d
	ld hl, wTargets ; A trigger ends any tone portamento.
	call ChannelWord
	xor a
	ld [hl+], a
	ld [hl], a
	ret

; Triggers pulse channel wChannel with the note in b.
TriggerPulse:
	call SetNotePeriod
	call PulseInstrument
	ld a, [hl]
	and $80 ; Length enable, bit 7 of instrument byte 0.
	rrca
	or $80
	ld d, a ; d = NRx4 without the period bits.
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
	call StoreVolume
	inc c
	call ChannelState
	inc hl ; CHANNEL_PERIOD
	ld a, [hl+]
	ldh [c], a ; NRx3
	inc c
	ld a, [hl]
	or d
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

; In: a = offset of a pointer in the song header (HDR_ORDER_TABLES or later).
; Out: hl = that pointer, read from wHeaderCache. Clobbers af (and keeps de).
SongHeaderWord:
	add LOW(wHeaderCache - HDR_ORDER_TABLES)
	ld l, a
	adc HIGH(wHeaderCache - HDR_ORDER_TABLES)
	sub l
	ld h, a
	ld a, [hl+]
	ld h, [hl]
	ld l, a
	ret

INCLUDE "notes.inc"
