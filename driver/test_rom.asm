; Driver test ROM: plays the song linked at `Song` and marks every frame for golem-run,
; which logs the APU writes between markers and times each driver call between a frame
; marker and the following end marker (tools/run/include/golem/rom_runner.h). Each call
; is exactly `ldh [FRAME_MARKER], a` / `call` / `ldh [CALL_END_MARKER], a`.

INCLUDE "golem.inc"

DEF FRAME_MARKER EQU $FF15 ; Unused APU address; any write starts a frame.
DEF CALL_END_MARKER EQU $FF27 ; Unused APU address; any write ends the timed call.
DEF rIF EQU $FF0F
DEF rIE EQU $FFFF
DEF IE_VBLANK EQU %00000001

SECTION "VBlank interrupt", ROM0[$40]
	reti

SECTION "Header", ROM0[$100]
	nop
	jp Start
	ds $150 - @, 0 ; Cartridge header, filled in by rgbfix

SECTION "Test ROM", ROM0
Start:
	di
	ld sp, $E000
	ld hl, Song
	ldh [FRAME_MARKER], a ; Frame 0
	call GolemInit
	ldh [CALL_END_MARKER], a

	ld a, IE_VBLANK
	ldh [rIE], a
	xor a
	ldh [rIF], a
	ei
.frame
	halt
	ldh [FRAME_MARKER], a ; Frames 1, 2, ...
	call GolemPlay
	ldh [CALL_END_MARKER], a
	jr .frame
